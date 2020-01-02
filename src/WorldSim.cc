#include "WorldSim.hh"

WorldSim::WorldSim(int width, int height)
  : food(width*height, 0), width(width), height(height) {
  initial_food_distribution();
}

unsigned int WorldSim::food_index(int x, int y) const {
  x = ((x%width) + width) % width;
  y = ((y%height) + height) % height;

  return y*width + x;
}

double& WorldSim::food_at(int x, int y) {
  return food.at(food_index(x,y));
}

double WorldSim::GetFoodAt(int x, int y) const {
  return food.at(food_index(x,y));
}

void WorldSim::initial_food_distribution() {
  food_at(width/2, height/2) = 1;
}

void WorldSim::iterate() {
  std::vector<double> prev_food = food;

  for(int x=0; x<width; x++) {
    for(int y=0; y<height; y++) {
      double ave_with_neighbors = (
        prev_food.at(food_index(x,y)) +
        prev_food.at(food_index(x+1,y)) +
        prev_food.at(food_index(x-1,y)) +
        prev_food.at(food_index(x,y-1)) +
        prev_food.at(food_index(x,y+1))
      ) / 5;

      food.at(food_index(x,y)) = ave_with_neighbors;
    }
  }
}
