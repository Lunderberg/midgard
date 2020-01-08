#pragma once

#include <string>
#include <vector>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "WorldController.hh"

class WorldSim;

class WebServer {
  typedef websocketpp::server<websocketpp::config::asio> server_t;

public:
  WebServer(WorldSim& sim);
  ~WebServer();

  void start(int port);

private:
  void on_http(websocketpp::connection_hdl hdl);

  void do_periodic_check();
  void on_first_message(websocketpp::connection_hdl handle,
                        server_t::message_ptr msg);
  void on_regular_message(websocketpp::connection_hdl handle,
                          server_t::message_ptr msg);

  void send_response(const ServerResponse& response);
  void broadcast_all(const std::string& message);

  server_t server;
  std::vector<websocketpp::connection_hdl> live_connections;
  std::string root_path;

  WorldController controller;
};
