#include "WebServer.hh"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "nlohmann/json.hpp"
using nlohmann::json;

#include "ProgramPath.hh"

WebServer::WebServer(WorldSim& sim)
  : server_running(false), stop_periodic_check(false), controller(sim) {

  //root_path = program_path() + "/web-serve";
  root_path = program_path() + "/../web-serve";

  server.set_http_handler(
    [this](websocketpp::connection_hdl hdl) { on_http(hdl); }
  );

  server.set_message_handler(
    [this](websocketpp::connection_hdl hdl, server_t::message_ptr msg) {
      on_first_message(hdl, msg);
    });

  server.clear_access_channels(websocketpp::log::alevel::all);
  server.clear_error_channels(websocketpp::log::elevel::all);
  server.set_reuse_addr(true);
}

WebServer::~WebServer() {
  server.stop_listening();
  for(auto& handle : live_connections) {
    websocketpp::lib::error_code ec;
    server.close(handle, websocketpp::close::status::going_away, "", ec);
  }
  stop_periodic_check = true;
  thread.join();
}

void WebServer::start(int port) {
  if(!server_running) {
    server_running = true;

    server.init_asio();
    server.listen(port);
    server.start_accept();
    do_periodic_check();
    thread = std::thread([this](){ run(); });
  }
}

void WebServer::run() {
  server.run();
}

namespace {
std::string cat_js_files(const std::string& dir) {
  std::vector<std::string> js_files;
  for(auto& p : std::filesystem::directory_iterator(dir)) {
    if(p.path().extension() == ".js") {
      js_files.push_back(p.path());
    }
  }

  std::sort(js_files.begin(), js_files.end());

  std::vector<char> text;
  for(auto& filename : js_files) {
    std::ifstream ifile(filename);
    text.insert(text.end(),
                std::istreambuf_iterator<char>(ifile),
                std::istreambuf_iterator<char>());
  }

  return std::string(text.begin(), text.end());
}

std::string read_file(const std::string& filename) {
  std::ifstream ifile(filename);
  return std::string(std::istreambuf_iterator<char>(ifile),
                     std::istreambuf_iterator<char>());
}
}

void WebServer::on_http(websocketpp::connection_hdl hdl) {
  auto con = server.get_con_from_hdl(hdl);

  if(con->get_resource().find("..") != std::string::npos) {
    con->set_status(websocketpp::http::status_code::forbidden);
    return;
  }


  if(con->get_resource() == "/world.js") {
    con->set_body(cat_js_files(root_path + "/js"));

  } else {
    std::string filename = root_path + con->get_resource();
    if(filename.back() == '/') {
      filename += "index.html";
    }
    con->set_body(read_file(filename));
  }
  con->set_status(websocketpp::http::status_code::ok);
}

void WebServer::on_first_message(websocketpp::connection_hdl handle,
                                  server_t::message_ptr msg) {
  std::cout << "Websocket connection received" << std::endl;

  auto payload = msg->get_payload();
  bool authenticated = true;

  if(authenticated) {
    auto conn = server.get_con_from_hdl(handle);
    conn->set_message_handler(
      [this](websocketpp::connection_hdl handle, server_t::message_ptr msg) {
        on_regular_message(handle, msg);
      });

    json j;
    j["authenticated"] = true;
    server.send(handle, j.dump(), websocketpp::frame::opcode::text);

    live_connections.push_back(handle);
  }
}

void WebServer::on_regular_message(websocketpp::connection_hdl handle,
                                    server_t::message_ptr msg) {
  auto response = controller.request(msg->get_payload(), handle);
  send_response(response);
}


void WebServer::do_periodic_check() {
  server.set_timer(
    100,
    [this](const websocketpp::lib::error_code& ec) {
      if(ec || stop_periodic_check) {
        return;
      }

      while(true) {
        auto response = controller.update_check();
        if(response.response.size() || response.broadcast.size()) {
          send_response(response);
        } else {
          break;
        }
      }

      do_periodic_check();
    });
}

void WebServer::send_response(const ServerResponse& response) {
  if(response.response.size()) {
    server.send(response.respond_to, response.response, websocketpp::frame::opcode::text);
  }
  if(response.broadcast.size()) {
    broadcast_all(response.broadcast);
  }
}

void WebServer::broadcast_all(const std::string& message) {
  auto send_msg = [&](auto& conn) {
    websocketpp::lib::error_code ec;
    this->server.send(conn, message, websocketpp::frame::opcode::text, ec);
    return ec;
  };


  live_connections.erase(
    std::remove_if(live_connections.begin(), live_connections.end(), send_msg),
    live_connections.end()
  );
}
