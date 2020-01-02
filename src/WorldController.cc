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

  if(j.count("iterate_n_steps") &&
     j["iterate_n_steps"] > 0) {
    int num_iter = j["iterate_n_steps"];

    for(int i=0; i<num_iter; i++) {
      sim.iterate();
    }
  }


  if(j.count("food_dist_requested") &&
     j["food_dist_requested"]) {
    output_reply["food_dist"] = get_food_dist();
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
