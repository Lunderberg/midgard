#include "WorldSim.hh"

#include "CreatureBrain_Wander.hh"

WorldSim::WorldSim(int num_layers, int random_seed)
  : food(num_layers), iterations_per_growth(4), iterations_since_growth(0),
    generator(random_seed) {
  initial_food_distribution();
  initial_creature_generation();
}

double WorldSim::GetFoodAt(int x, int y) const {
  return food.get_val(x,y);
}

std::vector<GrassyBitfield::DrawField> WorldSim::GetFoodDrawFields() const {
  return food.get_draw_fields();
}

void WorldSim::initial_food_distribution() {
  int num_seeds = 10;
  auto size = food.get_size();
  //Grr, uniform_int_distribution is inclusive at the top?  I get why,
  //but it is different from almost every other usage.
  std::uniform_int_distribution<std::uint32_t> dist(0, size-1);
  for(int i=0; i<num_seeds; i++) {
    food.set_val( dist(generator), dist(generator), true);
  }
}

void WorldSim::initial_creature_generation() {
  for(int i=0; i<1; i++) {
    auto brain = std::make_unique<CreatureBrain_Wander>();
    Creature new_creature(std::move(brain));
    new_creature.set_position({32,32});
    creatures.push_back(std::move(new_creature));
  }
}

void WorldSim::iterate() {
  if(iterations_since_growth >= iterations_per_growth) {
    food.growth_iteration();
    iterations_since_growth = 0;
  }
  iterations_since_growth++;

  for(auto& creature : creatures) {
    creature.update(generator, food);
  }
}
