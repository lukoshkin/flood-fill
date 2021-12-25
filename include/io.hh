#ifndef __IO_HH
#define __IO_HH
#include <fstream>

#include <mpi.h>
#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

namespace mpi = boost::mpi;


class ParallelRead {
  const char * fname;
  char * buf;

  public:
  ParallelRead(std::string f, char * data): fname(f.data()), buf(data) {}
  void read_mpi(MPI_Comm, MPI_Offset, MPI_Offset);
  void read_posix(uint, uint);
};


class ParallelWrite {
  const char * fname;

  public:
  ParallelWrite(std::string f) { fname = f.data(); }
  void write_mpi(MPI_Comm, char *, uint, uint);
};

#endif
