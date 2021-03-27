// The body of a class for conversion of raw file to mesh data.
// It also contains functions designed for extracting connected pores,
// where fluid can flow.

#ifndef __FLOOD_HH
#define __FLOOD_HH

#include "utils.hh"
#include "dsu.hh"


class Flood {
  uint Nx, Ny, Nz, N;

  uint8_t wall;
  uint8_t * data;

  DSU dsets;
  std::unordered_map<uint, std::vector<uint>> cavities;
  std::vector<uint> preceding_neighbor_positions, pad_pore_ids;
  std::vector<uint> flow_colors;

  void _colorFillDSU();
  void _flowDirColors(uint);
  void _screenFlowCavities();
  void _readData(std::string);
  void _copyData(uint8_t *);

public:

  Flood(
      uint, uint, uint,
      uint8_t * =NULL, std::string="",
      uint8_t=255, std::string="face");
  ~Flood();

  uint I2pI(uint);
  uint pI2I(uint);

  friend std::tuple<uint,uint,uint> multiIndex(uint, uint, uint);
  friend uint flatIndex(int, int, int, uint, uint);
  uint precedingNeighborPadId(uint, uint);

  void checkConnectivity(uint);
  void stats(uint, bool=false);

  std::vector<std::vector<uint>> flow_cavities;
  void selectFlowCavity(uint8_t *, uint=0);
  void screenFlowCavities();
};

#endif
