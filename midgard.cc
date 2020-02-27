#include <csignal>
#include <chrono>
#include <iostream>
#include <sstream>

#include "WebServer.hh"
#include "WorldSim.hh"

volatile sig_atomic_t keep_running = 1;

void signal_handler(int /*sig*/) {
  keep_running = 0;
}

int main() {
  WorldSim sim(2);
  sim.SetIterationsPerGrowth(20);

  WebServer server(sim);
  server.start(10101);

  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  while(keep_running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}
