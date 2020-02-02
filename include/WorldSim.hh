#pragma once

#include <random>
#include <vector>

#include "GrassyBitfield.hh"

class WorldSim {
public:
  WorldSim(int num_layers, int random_seed = 0);

  void iterate();

  int GetSize() const { return food.get_size(); }
  int GetNumLayers() const { return food.get_num_layers(); }

  double GetFoodAt(int x, int y) const;
  std::vector<GrassyBitfield::DrawField> GetFoodDrawFields() const;

private:
  void initial_food_distribution();

  GrassyBitfield food;
  std::mt19937 generator;
};
