#pragma once

#include <memory>
#include <random>

#include "GVector.hh"

class CreatureBrain;
class GrassyBitfield;

class Creature {
public:
  Creature(std::unique_ptr<CreatureBrain> new_brain);

  Creature(const Creature&) = delete;
  Creature(Creature&&) = default;
  Creature& operator=(const Creature&) = delete;
  Creature& operator=(Creature&&) = default;

  virtual ~Creature();

  void update(std::mt19937& gen, GrassyBitfield& field);

  GVector<2> get_position() const { return pos; }
  void set_position(GVector<2> pos) { this->pos = pos; }
  double get_direction() const;
  double get_speed() const { return speed; }
  double get_radius() const;

private:
  void default_updates(unsigned int world_size);
  void eat_food(GrassyBitfield& field);
  void turn(double delta_angle);
  void change_speed(double delta_v);

  double get_turn_angle();

  /// Position of the creature
  GVector<2> pos;
  /// Unit-vector with the direction the creature is facing
  GVector<2> direction;
  /// Current speed of the creature
  double speed;

  std::unique_ptr<CreatureBrain> brain;
};
