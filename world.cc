#include <iostream>
#include <sstream>

#include "WebServer.hh"

int main() {
  WebServer server;
  server.start(10101);
}
