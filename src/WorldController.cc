#include "WorldController.hh"

#include <chrono>
#include <iostream>


#include "WorldSim.hh"

WorldController::WorldController(WorldSim& sim)
  : sim(sim), worker_running(true) {
  sim_thread = std::thread([this](){worker_thread();});
}

WorldController::~WorldController() {
  worker_running = false;
  command_cv.notify_one();
  sim_thread.join();
}


ServerResponse WorldController::request(const std::string& command, std::weak_ptr<void> hdl) {
  try {
    return request_maythrow(command, hdl);
  } catch(std::domain_error& e) {
    std::cout << "Error in parsing: " << e.what() << "\n"
              << command << std::endl;
    return {"", "", hdl};
  }
}

ServerResponse WorldController::request_maythrow(const std::string& command, std::weak_ptr<void> hdl) {
  std::lock_guard<std::mutex> lock(command_mutex);
  worker_commands.push({command, hdl});
  command_cv.notify_one();

  // Cached items may later be returned from here without needing to
  // send to worker thread.
  return {"","",hdl};
}

void WorldController::worker_thread() {
  while(true) {
    std::unique_lock<std::mutex> lock(command_mutex);
    command_cv.wait(lock, [&]{
        return !worker_running || worker_commands.size();
      });

    if(!worker_running) {
      break;
    }

    Request req = std::move(worker_commands.front());
    worker_commands.pop();
    try{
      worker_thread_iter(req);
    } catch(std::domain_error& e) {
      std::cout << "Error in parsing: " << e.what() << "\n"
              << req.command << std::endl;
    }
  }
}

void WorldController::worker_thread_iter(Request& req) {
  auto j = json::parse(req.command);

  std::cout << "----------------------" << std::endl;
  std::cout << "Regular message received" << std::endl;
  std::cout << req.command << std::endl;

  json output_reply;
  json output_broadcast;

  if(j.count("reset_world") &&
     j["reset_world"]) {
    reset_world();
    broadcast_map_update();
  }

  if(j.count("iterate_n_steps") &&
     j["iterate_n_steps"] > 0) {
    int num_iter = j["iterate_n_steps"];

    for(int i=0; i<num_iter && worker_running; i++) {
      sim.iterate();
      broadcast_map_update();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  // If we are going to broadcast it to all connections, no need to
  // send an individual update as well.
  if(j.count("full_map_requested") &&
     j["full_map_requested"]) {
    output_reply["food_dist"] = get_food_dist();
    output_reply["creatures"] = get_creature_info();
  }

  std::lock_guard<std::mutex> lock(response_mutex);
  responses.push({ output_reply.empty() ? "" : output_reply.dump(),
        output_broadcast.empty() ? "" : output_broadcast.dump() ,
        req.requested_by});
}

ServerResponse WorldController::update_check() {
  std::lock_guard<std::mutex> lock(response_mutex);
  if(responses.size()) {
    auto output = std::move(responses.front());
    responses.pop();
    return output;
  } else {
    return {"", "", std::weak_ptr<void>()};
  }
}

void WorldController::broadcast_map_update() {
  json output_broadcast;
  output_broadcast["food_dist"] = get_food_dist();
  output_broadcast["creatures"] = get_creature_info();

  std::lock_guard<std::mutex> lock(response_mutex);
  responses.push({"", output_broadcast.dump(), std::weak_ptr<void>()});
}

json WorldController::get_food_dist() const {
  json output;

  output["size"] = sim.GetSize();

  std::vector<json> food_fields;
  for(const auto& field : sim.GetFoodDrawFields()) {
    json packed;
    packed["x_min"] = field.x_min;
    packed["y_min"] = field.y_min;
    packed["width"] = field.width;
    packed["values"] = field.values;
    food_fields.push_back(packed);
  }
  output["food_fields"] = food_fields;

  return output;
}

json WorldController::get_creature_info() const {
  std::vector<json> descriptions;

  for(const auto& creature : sim.GetCreatures()) {
    json desc;

    desc["x"] = creature.get_position().X();
    desc["y"] = creature.get_position().Y();
    desc["direction"] = creature.get_direction();
    desc["speed"] = creature.get_speed();
    desc["radius"] = creature.get_radius();

    descriptions.push_back(desc);
  }

  return json(descriptions);
}

void WorldController::reset_world() {
  sim = WorldSim(sim.GetNumLayers());
}
