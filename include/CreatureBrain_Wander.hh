#pragma once

#include "CreatureBrain.hh"

class CreatureBrain_Wander : public CreatureBrain {
public:
  virtual CreatureAction choose_action(std::mt19937& gen);
};
