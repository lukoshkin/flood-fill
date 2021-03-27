#include <cassert>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <boost/program_options.hpp>
#include "../include/flood.hh"

namespace po = boost::program_options;
namespace fs = std::filesystem;


int main(int argc, char * argv[]) {
  std::string fname;
  std::vector<uint> N;
  uint Nx, Ny, Nz;

  uint dir(2); 
  int wall(-1);
  std::string connectivity("face");
  assert(wall >= -255 && wall < 256);

  po::options_description desc("Allowed options");
  desc.add_options()
    ("input-file", po::value(&fname), "input file")
    ("help,h", "produce help message")
    ("flow-dir", po::value(&dir), "flow direction")
    ("dims,N", po::value(&N)->multitoken(),
     "size of cubic or rectangular shape in (one or three options possible)")
    ("connectivity", po::value(&connectivity),
     "pore connectivity: via vertex, edge, or face (default)")
    ("wall", po::value(&wall), "value of cell that is wall");

  po::positional_options_description pod;
  pod.add("input-file", 1).add("dims", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
      .options(desc).positional(pod).run(), vm);
  po::notify(vm);

  if (N.size() == 1) {
    Nx = N[0];
    Ny = Nz = Nx;
  } else if (N.size() == 3) {
    Nx = N[0];
    Ny = N[1];
    Nz = N[2];
  } else {
    std::cout << "Usage " << argv[0] << " <file.raw> <N> [options]\n\n";
    std::cout << desc;
    std::exit(1);
  }

  std::cout << "Search along direction: " << dir << std::endl;
  std::cout << "Wall identifier (available range is 0-255): ";
  std::cout << (unsigned)(uint8_t)wall << std::endl;
  std::cout << "Specified dimensions are";
  std::cout << " x:" << Nx << " y:" << Ny << " z:" << Nz << std::endl;

  Flood f(
      Nx, Ny, Nz,
      NULL, fname,
      wall, connectivity);


  f.checkConnectivity(dir);
  uint8_t * array = new uint8_t[Nx*Ny*Nz];
  f.selectFlowCavity(array, 0);
  f.stats(0, true);


  fs::path p(fname);
  fs::path newp = p.stem();
  std::string char_dir[3] = {"x", "y", "z"};
  newp += "_1c" + char_dir[dir] + p.extension().string();

  std::ofstream fp(p.parent_path()/newp, std::ios::binary);
  fp.write((const char*)array, Nx*Ny*Nz);
  fp.close();

  delete[] array;
}
