// The body of a class for conversion of raw file to mesh data.
// It also contains functions designed for extracting connected pores,
// where fluid can flow.

#ifndef __FLOOD_HH
#define __FLOOD_HH

#include <unordered_set>
#include <optional>

#include "utils.hh"
#include "dsu.hh"

using map_of_sets = std::unordered_map<uint, std::unordered_set<std::string>>;


class Flood {
  DSU dsets;
  uint offset, pad_offset;
  uint Nx, Ny, Nz, N;

  uint8_t wall;
  uint8_t * data;

  std::unordered_map<uint, std::vector<uint>> cavities;
  std::vector<uint> preceding_neighbor_positions, pad_pore_ids;
  map_of_sets flow_colors;

  void _readData(std::string);
  void _copyData(char *);

public:
  std::unordered_map<uint, std::vector<uint>> flow_cavities;

  Flood(
      uint, uint, uint,
      char * = NULL, std::string = "",
      uint8_t = 255, std::string = "face", uint = 0);
  ~Flood();

  uint I2pI(uint);
  uint pI2I(uint);

  friend std::tuple<uint,uint,uint> multiIndex(uint, uint, uint);
  friend uint flatIndex(int, int, int, uint, uint);
  uint precedingNeighborPadId(uint, uint);

  void checkConnectivity(uint);
  void stats(bool = false);

  void colorFillDSU();
  void linkToPrevDSU(std::vector<uint>);
  std::unordered_set<uint> faceColors(uint, uint);
  std::vector<uint> topFaceColors();
  map_of_sets flowPoreColors(
      std::vector<std::unordered_set<uint>>,
      std::vector<std::string>);
  void registerFlowCavities(char *, std::optional<map_of_sets> = {});
};

#endif
