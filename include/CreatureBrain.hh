#pragma once

#include <random>

#include "CreatureAction.hh"

class CreatureBrain {
public:
  virtual ~CreatureBrain() { }
  virtual CreatureAction choose_action(std::mt19937& gen) = 0;
};
