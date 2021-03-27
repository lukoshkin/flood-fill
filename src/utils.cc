#include <fstream>

#include <unordered_map>
#include <vector>
#include <tuple>


/**
 * Converts array id from 3D to 1D representation
 *
 * @param x - rel. (or abs., if cell_id > 0) coord along X axis.
 * @param y - rel. (or abs., if cell_id > 0) coord along Y axis.
 * @param z - rel. (or abs., if cell_id > 0) coord along Z axis.
 * @param bx - dimension size of X direction.
 * @param by - dimension size fo Y direction.
 */
uint flatIndex(int x, int y, int z, uint bx, uint by, uint cell_id/*=0*/) {
  return cell_id + z*by*bx + y*bx + x;
}


/**
 * Converts array id from 1D to cubic representation.
 * 
 * @param idx - target id which to convert in 3D representation.
 * @param b - size of a cube side.
 */
std::tuple<uint,uint,uint>
multiIndex(uint idx, uint bx, uint by) {
  uint z = idx / (by*bx);
  uint y = (idx - z*by*bx) / bx;
  uint x = idx - z*by*bx - y*bx;

  return std::make_tuple(x, y, z);
}


/**
 * Saves unordered map of <uint, std::vector<uint>> pairs
 * in a file that further can be parsed with Python.
 *
 * @param mapObj - std::unordered_map<uint, std::vector<uint>>.
 * @param fname - file where the map will be saved.
 */
using mapType = std::unordered_map<uint, std::vector<uint>>;
void saveMap(mapType& mapObj, std::string fname) {
  std::ofstream fp(fname, std::ios::binary);

  uint buf(mapObj.size());
  fp.write(reinterpret_cast<const char*>(&buf), sizeof(buf));

  for (auto it: mapObj)
    fp.write(reinterpret_cast<const char*>(&it.first), sizeof(uint));

  for (auto it: mapObj) {
    buf = it.second.size();
    fp.write(reinterpret_cast<const char*>(&buf), sizeof(buf));
  }
  for (auto it: mapObj)
    fp.write(reinterpret_cast<const char*>(
      &it.second[0]), sizeof(uint)*it.second.size());
  fp.close();
}

