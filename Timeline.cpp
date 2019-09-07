#include "Timeline.hpp"
#include <fstream>
#include <iostream>

Timeline::Timeline(std::string filepath){
  // from: https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c
  std::ifstream infile(filepath);
  float timepoint, value;
  while (infile >> timepoint >> value) {
    timepoints.push_back(timepoint);
    values.push_back(value);
  }
  std::cout << "a timeline created" << std::endl;
}

void Timeline::update(float elapsed) {
  if (!playing) return;

  current += elapsed;
  std::cout << "animation update.. current: " << current << std::endl;
  if (current >= til) {
    current = til;
    playing = false;
  }
}

Timeline::~Timeline() {
  std::cout << "timeline destructed." << std::endl;
}

void Timeline::set_interval(float from, float til) {
  current = from;
  this->til = til;
}

float Timeline::get_value() {
  if (timepoints.size() == 0) return 0.0f;
  if (current < timepoints[0]) return values[0];
  
  // return linear interpolated value bt. timepoints
  for(int i=0; i<timepoints.size() - 1; i++) {
    if (current >= timepoints[i]) {
      float v1 = values[i];
      float v2 = values[i+1];
      float segment_length = glm::max(epsilon, timepoints[i+1] - timepoints[i]);
      float time_offset = current - timepoints[i];
      float value_offset = (v2-v1) * (time_offset / segment_length);
      // cap value change to up to v2
      return value_offset > 0.0f ? 
        glm::min(v1+value_offset, v2) : glm::max(v1+value_offset, v2);
    }
  }
  return values[values.size()-1];
}
