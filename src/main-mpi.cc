#include <cassert>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <boost/mpi.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/program_options.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "../include/flood.hh"
#include "../include/io.hh"

namespace mpi = boost::mpi;
namespace po = boost::program_options;
namespace fs = std::filesystem;


int main(int argc, char * argv[]) {
  std::string fname;
  std::vector<uint> N;
  uint Nx, Ny, Nz;

  int wall(-1);
  std::string connectivity("face");
  assert(wall >= -255 && wall < 256);

  po::options_description desc("Allowed options");
  desc.add_options()
    ("input-file", po::value(&fname), "input file")
    ("help,h", "produce help message")
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

  mpi::environment env;
  mpi::communicator comm;
  uint prank = comm.rank();
  uint psize = comm.size();

  if (prank == 0) {
    std::cout << "Wall identifier (available range is 0-255): ";
    std::cout << (unsigned)(uint8_t)wall << std::endl;
    std::cout << "Specified dimensions are";
    std::cout << " x:" << Nx << " y:" << Ny << " z:" << Nz;
    std::cout << '\n' << std::endl;
  }

  uint Nz_loc = Nz / psize;
  uint offset_z = prank * Nz_loc;

  if (prank != 0) {
    --offset_z;
    ++Nz_loc;
  }

  uint bufsize = Nx*Ny*Nz_loc;
  uint offset = Nx*Ny*offset_z;

  /* std::cout << prank << std::endl; */
  /* std::cout << bufsize << std::endl; */
  /* std::cout << offset << std::endl; */
  /* std::cout << std::endl; */


  char * data = new char[bufsize];
  ParallelRead pr(fname, data);
  pr.read_mpi(comm, bufsize, offset);

  Flood f(
      Nx, Ny, Nz_loc,
      data, "", wall, connectivity, offset_z);

  f.colorFillDSU();

  if (psize > 1) {
    if (prank > 0) {
      std::vector<uint> get_colors;
      comm.irecv(prank-1, prank, get_colors);
      f.linkToPrevDSU(get_colors);
    }

    if (prank < psize-1) {
      std::vector<uint> pass_colors = f.topFaceColors();
      comm.isend(prank+1, prank, pass_colors);
    }
    comm.barrier();
  }
  std::vector<std::string> names(6);
  std::vector<std::unordered_set<uint>> faces(6);
  std::vector<std::unordered_set<uint>> face_chunks(psize);

  // Gather + Bcast
  map_of_sets flow_colors;
  for (uint i = 0; i < 6; ++i) {
    uint ax = i / 2;
    uint dir = i % 2;

    auto face = f.faceColors(ax, dir);
    gather(comm, face, face_chunks, 0);

    if (prank == 0) {
      names[i] = std::to_string(ax) + std::to_string(dir);

      for (auto & chunk : face_chunks)
        for (auto & elem : chunk)
          faces[i].emplace(elem);

      flow_colors = f.flowPoreColors(faces, names);
    }
    broadcast(comm, flow_colors, 0);
  }
  f.registerFlowCavities(data, flow_colors);

  // Allgather
  /* for (uint i = 0; i < 6; ++i) { */
  /*   uint ax = i / 2; */
  /*   uint dir = i % 2; */

  /*   auto face = f.faceColors(ax, dir); */
  /*   all_gather(comm, face, face_chunks); */

  /*   names[i] = std::to_string(ax) + std::to_string(dir); */

  /*   for (auto & chunk : face_chunks) */
  /*     for (auto & elem : chunk) */
  /*       faces[i].emplace(elem); */

  /*   f.flowPoreColors(faces, names); */
  /* } */
  /* f.registerFlowCavities(data); */

  if (prank != psize-1)
    bufsize -= Nx*Ny;

  /* std::cout << prank << std::endl; */
  /* std::cout << bufsize << std::endl; */
  /* std::cout << offset << std::endl; */
  /* std::cout << std::endl; */

  f.stats(true);

  ParallelWrite pw("connected.raw");
  pw.write_mpi(comm, data, bufsize, offset);

  /* fs::path p(fname); */
  /* fs::path newp = p.stem(); */
  /* std::string char_dir[3] = {"x", "y", "z"}; */
  /* newp += "_1c" + char_dir[dir] + p.extension().string(); */

  /* std::ofstream fp(p.parent_path()/newp, std::ios::binary); */
  /* fp.write((const char*)data, Nx*Ny*Nz); */
  /* fp.close(); */

  /* std::cout << "\nSaved to " << newp << std::endl; */

  delete [] data;
}
