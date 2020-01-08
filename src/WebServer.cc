#include "WebServer.hh"

#include <fstream>
#include <iostream>

#include "nlohmann/json.hpp"
using nlohmann::json;

#include "ProgramPath.hh"

WebServer::WebServer(WorldSim& sim)
  : controller(sim) {

  root_path = program_path();

  server.set_http_handler(
    [this](websocketpp::connection_hdl hdl) { on_http(hdl); }
  );

  server.set_message_handler(
    [this](websocketpp::connection_hdl hdl, server_t::message_ptr msg) {
      on_first_message(hdl, msg);
    });

  server.clear_access_channels(websocketpp::log::alevel::all);
  server.clear_error_channels(websocketpp::log::elevel::all);
}

WebServer::~WebServer() {
  // Need to make sure that it closes cleanly, but Ctrl-C interrupts
  // the server.run() command which handles the closing.  Will need to
  // find a better way to handle it in the future.


  // server.stop_listening();
  // for(auto& handle : live_connections) {
  //   websocketpp::lib::error_code ec;
  //   server.close(handle, websocketpp::close::status::going_away, "", ec);
  // }
}

void WebServer::start(int port) {
  server.init_asio();
  server.listen(port);
  server.start_accept();
  do_periodic_check();
  server.run();
}

void WebServer::on_http(websocketpp::connection_hdl hdl) {
  auto con = server.get_con_from_hdl(hdl);

  if(con->get_resource().find("..") != std::string::npos) {
    con->set_status(websocketpp::http::status_code::forbidden);
    return;
  }


  //std::string filename = root_path + "/web-serve" + con->get_resource();
  std::string filename = root_path + "/../web-serve" + con->get_resource();
  if(filename.back() == '/') {
    filename += "index.html";
  }



  std::ifstream ifile(filename);
  auto content = std::string(std::istreambuf_iterator<char>(ifile),
                             std::istreambuf_iterator<char>());

  con->set_status(websocketpp::http::status_code::ok);
  con->set_body(content);
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
      if(ec) {
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
