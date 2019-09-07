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
}

void Timeline::update(float elapsed) {
  if (!playing) return;

  current += elapsed;
  if (current >= til) {
    current = til;
    playing = false;
  }
}

Timeline::~Timeline() {
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
    if (current >= timepoints[i] && current < timepoints[i+1]) {
      float v1 = values[i];
      float v2 = values[i+1];
      float segment_length = glm::max(epsilon, timepoints[i+1] - timepoints[i]);
      // cap time offset to be at most at segment endpoint
      float time_offset = glm::min(current - timepoints[i], segment_length);
      float value_offset = (v2-v1) * (time_offset / segment_length);
      return v1 + value_offset;
    }
  }
  return values[values.size()-1];
}
