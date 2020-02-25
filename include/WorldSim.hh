#pragma once

#include <random>
#include <vector>

#include "GrassyBitfield.hh"
#include "Creature.hh"

class WorldSim {
public:
  WorldSim(int num_layers, int random_seed = 0);

  void iterate();

  int GetSize() const { return food.get_size(); }
  int GetNumLayers() const { return food.get_num_layers(); }

  double GetFoodAt(int x, int y) const;
  std::vector<GrassyBitfield::DrawField> GetFoodDrawFields() const;

  int GetIterationsPerGrowth() const { return iterations_per_growth; }
  void SetIterationsPerGrowth(int new_rate) { iterations_per_growth = new_rate; }

  const std::vector<Creature>& GetCreatures() { return creatures; }

private:
  void initial_food_distribution();
  void initial_creature_generation();

  GrassyBitfield food;
  int iterations_per_growth;
  int iterations_since_growth;

  std::vector<Creature> creatures;
  std::mt19937 generator;
};
