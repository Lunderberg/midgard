#include <iostream>
#include <sstream>

#include "WebServer.hh"
#include "WorldSim.hh"

int main() {
  //WorldSim sim(1000, 1000);
  WorldSim sim(10, 10);

  WebServer server(sim);
  server.start(10101);
}
