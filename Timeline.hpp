#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Timeline {

  Timeline(std::string filepath);
  ~Timeline();

  float current = 0.0f;
  float til;
  bool playing = false;
  void update(float elapsed); 

  void set_interval(float from, float til);
  float get_value();

  std::vector<float> timepoints;
  std::vector<float> values;
  float epsilon = 0.01f;
  
};
