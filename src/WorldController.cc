#include "WorldController.hh"

#include <iostream>


#include "WorldSim.hh"

WorldController::WorldController(WorldSim& sim)
  : sim(sim) { }


ServerResponse WorldController::request(const std::string& command) {
  try {
    return request_maythrow(command);
  } catch(std::domain_error& e) {
    std::cout << "Error in parsing: " << e.what() << "\n"
              << command << std::endl;
    return {"", ""};
  }
}

ServerResponse WorldController::request_maythrow(const std::string& command) {
  std::cout << "----------------------" << std::endl;
  std::cout << "Regular message received" << std::endl;
  std::cout << command << std::endl;

  auto j = json::parse(command);

  json output_reply;
  json output_broadcast;

  bool map_update_needed = false;

  if(j.count("reset_world") &&
     j["reset_world"]) {
    reset_world();
    map_update_needed = true;
  }

  if(j.count("iterate_n_steps") &&
     j["iterate_n_steps"] > 0) {
    int num_iter = j["iterate_n_steps"];

    for(int i=0; i<num_iter; i++) {
      sim.iterate();
    }
    map_update_needed = true;
  }

  // If we are going to broadcast it to all connections, no need to
  // send an individual update as well.
  if(j.count("food_dist_requested") &&
     j["food_dist_requested"] &&
     !map_update_needed) {
    output_reply["food_dist"] = get_food_dist();
  }

  if(map_update_needed) {
    output_broadcast["food_dist"] = get_food_dist();
  }

  return { output_reply.empty() ? "" : output_reply.dump(),
      output_broadcast.empty() ? "" : output_broadcast.dump() };
}

json WorldController::get_food_dist() const {
  json output;

  int width = sim.GetWidth();
  int height = sim.GetHeight();

  output["width"] = width;
  output["height"] = height;

  std::vector<std::vector<double> > food;
  food.reserve(width);

  for(int x=0; x<sim.GetWidth(); x++) {
    std::vector<double> col;
    col.reserve(height);

    for(int y=0; y<sim.GetHeight(); y++) {
      col.push_back(sim.GetFoodAt(x,y));
    }
    food.push_back(col);
  }
  output["food"] = food;

  return output;
}

void WorldController::reset_world() {
  sim = WorldSim(sim.GetWidth(), sim.GetHeight());
}
