// Some basic functions used in the raw2mesh converter

#ifndef __UTILS_HH
#define __UTILS_HH

#include <unordered_map>
#include <vector>

std::tuple<uint,uint,uint> multiIndex(uint, uint, uint);
uint flatIndex(int, int, int, uint, uint, uint=0);

void saveMap(
  std::unordered_map<uint, std::vector<uint>>&,
  std::string);

#endif
