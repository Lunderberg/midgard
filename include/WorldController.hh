#pragma once

#include <string>

#include "nlohmann/json.hpp"
using nlohmann::json;

class WorldSim;

struct ServerResponse {
  std::string response;
  std::string broadcast;
};

class WorldController {
public:
  WorldController(WorldSim& sim);

  ServerResponse request(const std::string& command);

private:
  ServerResponse request_maythrow(const std::string& command);
  json get_food_dist() const;

  WorldSim& sim;
};
