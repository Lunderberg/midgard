#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "nlohmann/json.hpp"
using nlohmann::json;

class WorldSim;

struct ServerResponse {
  std::string response;
  std::string broadcast;
  std::weak_ptr<void> respond_to;
};

class WorldController {
  struct Request {
    std::string command;
    std::weak_ptr<void> requested_by;
  };
public:
  WorldController(WorldSim& sim);
  ~WorldController();

  ServerResponse request(const std::string& command, std::weak_ptr<void> hdl);
  ServerResponse update_check();

private:
  ServerResponse request_maythrow(const std::string& command, std::weak_ptr<void> hdl);

  void worker_thread();
  void worker_thread_iter(Request& req);

  // To be called only from worker thread
  void broadcast_map_update();
  json get_food_dist() const;
  void reset_world();


  WorldSim& sim;


  std::atomic_bool worker_running;
  std::thread sim_thread;

  std::mutex command_mutex;
  std::condition_variable command_cv;
  std::queue<Request> worker_commands;

  std::mutex response_mutex;
  std::queue<ServerResponse> responses;
};
