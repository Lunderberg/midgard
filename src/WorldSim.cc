#include "WorldSim.hh"

WorldSim::WorldSim(int num_layers, int random_seed)
  : food(num_layers), generator(random_seed) {
  initial_food_distribution();
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

void WorldSim::iterate() {
  food.growth_iteration();
}
