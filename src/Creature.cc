#include "Creature.hh"

#include <algorithm>
#include <cmath>

#include "CreatureBrain.hh"
#include "GrassyBitfield.hh"

namespace{
  const double acceleration = 0.1;
  const double turn_speed = 10 * (3.1415926/180);
  const double speed_decay = 0.95;
  const double creature_radius = 4;
}

Creature::Creature(std::unique_ptr<CreatureBrain> new_brain)
  : pos(0,0), direction(1,0), speed(0),
    brain(std::move(new_brain)) { }

Creature::~Creature() { }

double Creature::get_direction() const {
  return std::atan2(direction.Y(), direction.X());
}

double Creature::get_radius() const {
  return creature_radius;
}

void Creature::update(std::mt19937& gen, GrassyBitfield& field) {
  CreatureAction action = brain->choose_action(gen);
  switch(action) {
    case CreatureAction::NoAction:
      break;

    case CreatureAction::EatFood:
      eat_food(field);
      break;

    case CreatureAction::TurnLeft:
      turn(+get_turn_angle());
      break;

    case CreatureAction::TurnRight:
      turn(-get_turn_angle());
      break;

    case CreatureAction::SpeedUp:
      change_speed(+acceleration);
      break;

    case CreatureAction::SlowDown:
      change_speed(-acceleration);
      break;
  }

  default_updates(field.get_size());
}

void Creature::default_updates(unsigned int world_size) {
  pos += speed*direction;
  pos.X() = std::fmod(pos.X() + world_size, world_size);
  pos.Y() = std::fmod(pos.Y() + world_size, world_size);

  speed *= speed_decay;
}

void Creature::turn(double delta_angle) {
  double new_angle = get_direction() + delta_angle;
  direction = GVector<2>(std::cos(new_angle), std::sin(new_angle));
}

double Creature::get_turn_angle() {
  return turn_speed / std::max(1.0, speed);
}

void Creature::change_speed(double delta_v) {
  speed = std::max(0.0, speed + delta_v);
}

void Creature::eat_food(GrassyBitfield& field) {
  // If needed, this search could be moved to GrassyBitfield.  The
  // implementation would be more complicated, but could be much more
  // efficient there.
  using tile = std::tuple<int, int>;

  auto dist2 = [&](const tile& t) {
    return (std::pow(std::get<0>(t)-0.5 - pos.X(), 2) +
            std::pow(std::get<1>(t)-0.5 - pos.Y(), 2));
  };

  std::vector<tile> nearby;
  for(int di = -creature_radius; di <= +creature_radius; di++) {
    for(int dj = -creature_radius; dj <= +creature_radius; dj++) {
      tile new_tile{pos.X()+di, pos.Y()+dj};
      if(dist2(new_tile) < creature_radius*creature_radius) {
        nearby.push_back(new_tile);
      }
    }
  }

  std::sort(nearby.begin(), nearby.end(),
            [&](const tile& a, const tile& b) {
              return dist2(a) < dist2(b);
            });

  for(auto loc : nearby) {
    int x = std::get<0>(loc);
    int y = std::get<1>(loc);
    if(field.get_val(x,y)) {
      field.set_val(x, y, false);
      break;
    }
  }
}
