#pragma once

#include <memory>
#include <vector>

class WorldSim {
public:
  WorldSim(int width, int height);

  void iterate();

  int GetWidth() const { return width; }
  int GetHeight() const { return height; }

  double GetFoodAt(int x, int y) const;

private:
  void initial_food_distribution();

  unsigned int food_index(int x, int y) const;
  double& food_at(int x, int y);

  std::vector<double> food;
  int width;
  int height;
};
