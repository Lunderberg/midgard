#pragma once

#include <string>
#include <vector>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

class WebServer {
  typedef websocketpp::server<websocketpp::config::asio> server_t;

public:
  WebServer();
  void start(int port);

private:
  void on_http(websocketpp::connection_hdl hdl);

  void on_first_message(websocketpp::connection_hdl handle,
                        server_t::message_ptr msg);
  void on_regular_message(websocketpp::connection_hdl handle,
                          server_t::message_ptr msg);

  void broadcast_all(const std::string& message);

  server_t server;
  std::vector<websocketpp::connection_hdl> live_connections;
  std::string root_path;
};
