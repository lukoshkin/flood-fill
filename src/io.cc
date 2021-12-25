#include "../include/io.hh"
#define CHECK(fn) { int errcode; errcode = (fn);\
     if (errcode != MPI_SUCCESS) handle_error  (errcode, #fn ); }

static void handle_error(int errcode, const char *str) {
    char msg[MPI_MAX_ERROR_STRING];
    int resultlen;
    MPI_Error_string(errcode, msg, &resultlen);
    fprintf(stderr, "%s: %s\n", str, msg);
    MPI_Abort(MPI_COMM_WORLD, 1);
}


void ParallelRead::read_posix(uint bufsize, uint offset) {
  std::ifstream fp(fname);
  fp.seekg(offset, fp.beg);
  fp.read(buf, bufsize*sizeof(char));
  fp.close();
}

/* void ParallelRead::read_posix(uint bufsize, uint offset) { */
/*   std::ifstream fp(fname); */
/*   assert(fp.is_open()); */
/*   fp.seekg(offset, fp.beg); */

/*   char tmp; */
/*   for (uint i = 0; i < bufsize; ++i) { */
/*     fp.read(&tmp, sizeof(char)); */
/*     buf[i] = tmp; */
/*   } */

/*   fp.close(); */
/* } */

/* void ParallelRead::read_mpi(MPI_Comm comm, uint bufsize, uint offset) { */
/*   MPI_File fp; */
/*   MPI_Comm world(comm); */
/*   MPI_File_open( */
/*       world, fname, */
/*       MPI_MODE_RDONLY, */
/*       MPI_INFO_NULL, &fp); */

/*   MPI_File_set_view( */
/*       fp, offset*sizeof(char), MPI_CHAR, */
/*       MPI_CHAR, "native", MPI_INFO_NULL); */
/*   MPI_File_read_all(fp, buf, bufsize, MPI_CHAR, MPI_STATUS_IGNORE); */
/*   MPI_File_close(&fp); */
/* } */

void ParallelRead::read_mpi(
    MPI_Comm comm, MPI_Offset bufsize, MPI_Offset offset) {

  MPI_File fp;
  MPI_Comm world(comm);

  std::cout << "FROBBED FILENAME: " << fname << std::endl;
  CHECK(MPI_File_open(
      world, fname,
      MPI_MODE_RDONLY,
      MPI_INFO_NULL, &fp));

  MPI_File_read_at_all(
      fp, offset*sizeof(char), buf,
      bufsize, MPI_CHAR, MPI_STATUS_IGNORE);

  MPI_File_close(&fp);
}

void ParallelWrite::write_mpi(
    MPI_Comm comm, char * buf,
    uint bufsize, uint offset) {

  MPI_File fp;
  MPI_Comm world(comm);
  MPI_File_open(
      world, fname,
      MPI_MODE_CREATE | MPI_MODE_WRONLY,
      MPI_INFO_NULL, &fp);

  MPI_File_set_view(
      fp, offset*sizeof(char), MPI_CHAR,
      MPI_CHAR, "native", MPI_INFO_NULL);
  MPI_File_write_all(fp, buf, bufsize, MPI_CHAR, MPI_STATUS_IGNORE);
  MPI_File_close(&fp);
}
