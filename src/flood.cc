#include <fstream>
#include <iostream>

#include <cmath>
#include <cassert>
#include <unordered_set>
#include <set>

#include "../include/flood.hh"


/**
 * Algorithm for finding the connected components
 * in the chosen flow direction (3D task).
 *
 * @param dimAlpha - size of dim Alpha (where Alpha = {x,y,z})
 * @param[in] array - input data in uint8_t format.
 * @param file - name of a binary file with segmented pores.
 * @param wall_id - value of wall identifier in the input data.
 * @param connectivity - type of pores connectivity.
 */
Flood::Flood(
    uint dimx, uint dimy, uint dimz,
    uint8_t * array/*=NULL*/, std::string file/*=""*/,

    uint8_t wall_id/*=255*/,
    std::string connectivity/*="face"*/) {

  assert(dimx > 0);
  assert(dimy > 0);
  assert(dimz > 0);

  if (connectivity == "face")
    preceding_neighbor_positions = {4,10,12};
  else if (connectivity == "edge")
    preceding_neighbor_positions = {1,3,4,5,7,10,12};
  else if (connectivity == "vertex")
    preceding_neighbor_positions = {0,1,2,3,4,5,6,7,8,9,10,11,12};
  else throw std::invalid_argument(
      "possible values are: vertex, edge or face");

  dsets = DSU();

  Nx = dimx;
  Ny = dimy;
  Nz = dimz;
  wall = wall_id;

  N = Nx*Ny*Nz;
  uint Np = (Nx+1)*(Ny+1)*(Nz+1);

  data = new uint8_t[Np];
  for (uint i=0; i<Np; ++i) data[i] = wall;

  if (array != NULL) _copyData(array);
  else if (!file.empty()) _readData(file);
  else throw std::invalid_argument(
      "At least one of the input options must be specified");
}

Flood::~Flood() {
  delete[] data;
}

/**
 * Reads binary data from a raw file.
 *
 * @param file - file name.
 */
void Flood::_readData(std::string file) {
  std::ifstream fp(file);
  assert(fp.is_open());

  char buf;
  for (uint cid=0; fp.read(&buf, sizeof(char)); ++cid) {
    uint pcid = I2pI(cid);
    data[pcid] = (uint8_t)buf;
    if (data[pcid] != wall)
      pad_pore_ids.push_back(pcid);
  }
  fp.close();
}

/**
 * Copies data from an array.
 *
 * @param array - uint8_t array representing input data
 */
void Flood::_copyData(uint8_t * array) {
  for (uint i=0; i<N; ++i) {
    uint cid = I2pI(i);
    data[cid] = array[i];
    if (array[i] != wall)
      pad_pore_ids.push_back(cid);
  }
}


/**
 * Maps indices in original and padded arrays.
 *
 * @param cell_id - id of a cell in the original array.
 * @return The index in the padded array.
 */
uint Flood::I2pI(uint cell_id) {
  auto [x, y, z] = multiIndex(cell_id, Nx, Ny);
  return flatIndex(x+1, y+1, z+1, Nx+1, Ny+1);
}

/**
 * Maps indices in original and padded arrays (opposite direction).
 *
 * @param cell_id - id of a cell in the padded array.
 * @return The index in the original array.
 */
uint Flood::pI2I(uint cell_id) {
  auto [x, y, z] = multiIndex(cell_id, Nx+1, Ny+1);
  return flatIndex(x-1, y-1, z-1, Nx, Ny);
}

/**
 * Finds preceding neighbor cell index in the padded array
 * by its relative position to the target cell.
 *
 * @param pad_cell_id - index of the target cell in the padded array @see data.
 * @param pos - relative position of the neighbor cell with a lesser index.
 * @return The index of a preceding neighbor cell.
 */
uint Flood::precedingNeighborPadId(uint pad_cell_id, uint pos) {
  auto [dx, dy, dz] = multiIndex(pos, 3, 3);
  return flatIndex(dx-1, dy-1, dz-1, Nx+1, Ny+1, pad_cell_id);
}

/**
 * Fills cavities with different colors.
 * Logs ids of segmented pores to "cavities.bin".
 */
void Flood::_colorFillDSU() {
  for (uint ppid: pad_pore_ids) {
    dsets.makeSet(ppid);

    for (uint pos: preceding_neighbor_positions) {
      uint pnpid = precedingNeighborPadId(ppid, pos);
      if (data[pnpid] != wall) dsets.unionSets(ppid, pnpid);
    }
  }
  for (uint ppid: pad_pore_ids)
    cavities[dsets.findSet(ppid)].push_back(pI2I(ppid));
}

/**
 * Compares colors of pores on the bottom and top faces,
 * selecting common ones and logging them to "target_pores.bin".
 */
using unord_set = std::unordered_set<uint>;
void Flood::_flowDirColors(uint flow_dir) {
  assert(!cavities.empty());
  std::vector<uint> I, DIM;

  switch (flow_dir) {
    case 0:
      I = {0, 1, 2};
      DIM = {Nx+1, Ny+1, Nz+1};
      break;
    case 1:
      I = {1, 0, 2};
      DIM = {Ny+1, Nx+1, Nz+1};
      break;
    case 2:
      I = {1, 2, 0};
      DIM = {Nz+1, Nx+1, Ny+1};
      break;
    default:
      throw "flow_dir takes only 0, 1, 2 values";
  }

  unord_set top_face_pores;
  unord_set bottom_face_pores;

  for (uint i=1; i<DIM[1]; ++i) {
    for (uint j=1; j<DIM[2]; ++j) {

      std::vector<uint> B = {1, i, j};
      uint bottom_cell_id = flatIndex(
          B[I[0]], B[I[1]], B[I[2]], Nx+1, Ny+1);

      if (data[bottom_cell_id] != wall)
        bottom_face_pores.emplace(
          dsets.findSet(bottom_cell_id));

      std::vector<uint> T = {DIM[0]-1, i, j};
      uint top_cell_id = flatIndex(
          T[I[0]], T[I[1]], T[I[2]], Nx+1, Ny+1);

      if (data[top_cell_id] != wall)
        top_face_pores.emplace(
          dsets.findSet(top_cell_id));
    }
  }
  unord_set * smaller_face = &top_face_pores;
  unord_set * bigger_face = &bottom_face_pores;
  if (smaller_face->size() > bigger_face->size())
    std::swap(smaller_face, bigger_face);

  flow_colors.clear();
  for (auto it=smaller_face->begin(); it!=smaller_face->end(); ++it)
    if (bigger_face->find(*it) != bigger_face->end())
      flow_colors.push_back(*it);
}

/**
 * Checks the sample connectivity in the given direction
 *
 * @param flow_dir - flow direction
 */
void Flood::checkConnectivity(uint flow_dir) {
  _colorFillDSU();
  _flowDirColors(flow_dir);
  assert(!flow_colors.empty());
}

/**
 * Filter out cavities, leaving only flow ones.
 * @note The result of this function depends on the flow_dir
 * argument chosen in @see checkConnectivity function.
 */
void Flood::screenFlowCavities() {
  assert(!cavities.empty());
  assert(!flow_colors.empty());

  struct BiggerVecSize {
    bool operator() (
        const std::vector<uint>& a,
        const std::vector<uint>& b) const {
      return a.size() > b.size();
    }
  } comp;

  flow_cavities.clear();
  for (uint color: flow_colors) flow_cavities.push_back(cavities[color]);
  std::sort(flow_cavities.begin(), flow_cavities.end(), comp);
}

/**
 * Fills the provided array with the selected flow component.
 * @note The result of this function depends on the flow_dir
 * argument chosen in @see checkConnectivity function.
 *
 * @param[out] array - where to write to connected component.
 * @param kth - number of the selected flow cavity.
 */
void Flood::selectFlowCavity(uint8_t * array, uint kth/*=0*/) {
  if (flow_cavities.empty()) {
    screenFlowCavities();
    assert(!flow_cavities.empty());
  }

  uint CNT(0);
  for (uint ppid: pad_pore_ids) {
    if (CNT >= flow_cavities[kth].size()) return;

    uint pid = pI2I(ppid);
    if (pid >= flow_cavities[kth][CNT]) CNT++;
    else array[pid] = wall;
  }
}

/**
 * Prints the information about the number of pores found
 * and the size of a selected flow cavity.
 *
 * @kth - the k-th largest flow cavity.
 * @full - whether to print the total number of pores.
 */
void Flood::stats(uint kth, bool full/*=false*/) {
  if (full) {
    std::cout << "total # of pores: ";
    std::cout << pad_pore_ids.size() << '\n';
  }

  if (!flow_cavities.empty()) {
    std::cout << "# of pores in the " << kth <<  " cavity: ";
    std::cout << flow_cavities[kth].size() << std::endl;
  }
}

