#include "CreatureBrain_Wander.hh"

CreatureAction CreatureBrain_Wander::choose_action(std::mt19937& gen) {
  std::uniform_real_distribution<> dis(0,1);
  double rand = dis(gen);

  if(rand < 0.30) {
    return CreatureAction::SpeedUp;
  } else if (rand < 0.45) {
    return CreatureAction::TurnLeft;
  } else if (rand < 0.60) {
    return CreatureAction::TurnRight;
  } else {
    return CreatureAction::EatFood;
  }
}
