#include <fstream>
#include <iostream>

#include <cassert>
#include <cmath>
#include <set>

#include "../include/flood.hh"


/**
 * Algorithm for finding the connected components
 * in the chosen flow direction (3D task).
 *
 * @param dimAlpha - size of dim Alpha (where Alpha = {x,y,z})
 * @param[in] array - input data in char format.
 * @param file - name of a binary file with segmented pores.
 * @param wall_id - value of wall identifier in the input data.
 * @param connectivity - type of pores connectivity.
 */
Flood::Flood(
    uint dimx, uint dimy, uint dimz,
    char * array/* = NULL*/, std::string file/* = ""*/,

    uint8_t wall_id/* = 255*/,
    std::string connectivity/* = "face"*/, uint offset_z/* = 0*/) {

  assert(dimx > 0);
  assert(dimy > 0);
  assert(dimz > 0);

  if (connectivity == "face")
    preceding_neighbor_positions = {4,10,12};
  else if (connectivity == "edge")
    preceding_neighbor_positions = {1,3,4,5,7,9,10,11,12};
  else if (connectivity == "vertex")
    preceding_neighbor_positions = {0,1,2,3,4,5,6,7,8,9,10,11,12};
  else throw std::invalid_argument(
      "possible values are: vertex, edge or face");

  Nx = dimx;
  Ny = dimy;
  Nz = dimz;
  wall = wall_id;

  dsets = DSU();
  pad_offset = (Nx+1) * (Ny+1) * offset_z;

  N = Nx * Ny * Nz;
  uint Np = (Nx+1) * (Ny+1) * (Nz+1);

  data = new uint8_t[Np];
  for (uint i = 0; i < Np; ++i) data[i] = wall;

  if (array != NULL) _copyData(array);
  else if (!file.empty()) _readData(file);
  else throw std::invalid_argument(
      "At least one of the input options must be specified");
}

Flood::~Flood() {
  delete[] data;
}

/**
 * Read binary data from a raw file.
 *
 * @param file - file name.
 */
void Flood::_readData(std::string file) {
  std::ifstream fp(file);
  assert(fp.is_open());

  char buf;
  for (uint i = 0; fp.read(&buf, sizeof(char)); ++i) {
    uint pcid = I2pI(i);
    data[pcid] = (uint8_t)buf;
    if (data[pcid] != wall)
      pad_pore_ids.push_back(pcid + pad_offset);
  }
  fp.close();
}

/**
 * Copy data from an array.
 *
 * @param array - char array representing input data
 */
void Flood::_copyData(char * array) {
  for (uint i = 0; i < N; ++i) {
    uint pcid = I2pI(i);
    data[pcid] = (uint8_t)array[i];
    if (data[pcid] != wall) {
      pad_pore_ids.push_back(pcid + pad_offset);
    }
  }
}

/**
 * Map indices in original and padded arrays.
 *
 * @param cell_id - id of a cell in the original array.
 * @return The index in the padded array.
 */
uint Flood::I2pI(uint cell_id) {
  auto [x, y, z] = multiIndex(cell_id, Nx, Ny);
  return flatIndex(x+1, y+1, z+1, Nx+1, Ny+1);
}

/**
 * Map indices in original and padded arrays (opposite direction).
 *
 * @param cell_id - id of a cell in the padded array.
 * @return The index in the original array.
 */
uint Flood::pI2I(uint cell_id) {
  auto [x, y, z] = multiIndex(cell_id, Nx+1, Ny+1);
  return flatIndex(x-1, y-1, z-1, Nx, Ny);
}

/**
 * Find preceding neighbor cell index in the padded array
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
 * Fill cavities with different colors.
 */
void Flood::colorFillDSU() {
  for (uint ppid : pad_pore_ids) {
    dsets.makeSet(ppid);

    for (uint pos : preceding_neighbor_positions) {
      uint pnpid = precedingNeighborPadId(ppid, pos);
      if (data[pnpid - pad_offset] != wall)
        dsets.unionSets(ppid, pnpid);
    }
  }
}

/**
 *
 */
std::vector<uint> Flood::topFaceColors () {
  std::vector<uint> top_face_colors;
  uint startId = (Nx+1)*(Ny+1)*(Nz-1) + pad_offset;
  for (auto i = pad_pore_ids.rbegin(); *i >= startId; ++i)
    top_face_colors.push_back(dsets.findSet(*i)->data);

  return top_face_colors;
}

/**
 * Set parents of pores from external DSU to those of `this->dsets`.
 */
void Flood::linkToPrevDSU(std::vector<uint> top_face_colors) {
  uint j(0);
  uint stopId = (Nx+1)*(Ny+1) + pad_offset;
  for (auto i = pad_pore_ids.begin(); *i < stopId; ++i)
    dsets.changeParent(*i, top_face_colors[j++]);
}

/**
 * Returns pore colors on the specified with `ax` and `dir` face.
 */
std::unordered_set<uint>
Flood::faceColors(uint ax, uint dir) {
  assert(ax < 3);
  assert(dir < 2);
  std::vector<uint> I, DIM;

  switch (ax) {
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
  }

  uint plane_id = (dir > 0) ? DIM[0]-1 : 1;
  std::unordered_set<uint> face_colors;

  for (uint i = 1; i < DIM[1]; ++i) {
    for (uint j = 1; j < DIM[2]; ++j) {

      std::vector<uint> B = {plane_id, i, j};
      uint face_cid = flatIndex(
          B[I[0]], B[I[1]], B[I[2]], Nx+1, Ny+1);

      if (data[face_cid] != wall) {
        face_cid += pad_offset;
        face_colors.emplace(dsets.findSet(face_cid)->data);
      }
    }
  }
  return face_colors;
}


/**
 * Find colors of flow pores (that start on one face and end on another)
 */
map_of_sets Flood::flowPoreColors(
    std::vector<std::unordered_set<uint>> faces,
    std::vector<std::string> face_names) {

  for (uint i = 0; i < faces.size(); ++i) {
    for (auto & color : faces[i])
      flow_colors[color].emplace(face_names[i]);
  }

  for (auto it = flow_colors.begin(); it != flow_colors.end();) {
    if (it->second.size() > 1)
      it = flow_colors.erase(it);
    else
      ++it;
  }
  return flow_colors;
}

/**
 * @param data is original array with data passed to the constructor.
 */
void Flood::registerFlowCavities(
    char * data, std::optional<map_of_sets> fc /* = {} */) {
  map_of_sets flow_colors = fc.value_or(this->flow_colors);

  for (uint ppid : pad_pore_ids) {
    uint color = dsets.findSet(ppid)->data;
    uint pid = pI2I(ppid - pad_offset);

    if (flow_colors.count(color))
      flow_cavities[color].push_back(pid);
    else
      data[pid] = (char)wall;
  }
}

/**
 * Prints the information about the number of pores found
 * and the size of a selected flow cavity.
 *
 * @full - whether to print the total number of pores.
 */
void Flood::stats(bool full/* = false*/) {
  std::cout.precision(4);
  if (full) {
    std::cout << "total # of pores: ";
    std::cout << pad_pore_ids.size() << '\n';
    std::cout << "Porosity = ";
    std::cout << 100*(float)pad_pore_ids.size() / N;
    std::cout << "%\n";
  }


  if (!flow_cavities.empty()) {
    uint cnt(0);
    for (auto & fc : flow_cavities)
      cnt += fc.second.size();

    std::cout << "# of cavities: ";
    std::cout << flow_colors.size() << '\n';
    std::cout << "Truncated sample porosity = ";
    std::cout << 100*(float)cnt / N;
    std::cout << '%' << std::endl;
  }
}

