/*
 *  This example writes data to the HDF5 file by rows.
 *  Number of processes is assumed to be 1 or multiples of 2 (up to 8)
 */

#include "hdf5.h"
#include "stdlib.h"
#include <iostream>

#include <random>
#include <vector>

#define H5FILE_NAME "SDS_row.h5"
#define DATASETNAME "IntVector"

#include <unistd.h>
unsigned int microsecond = 1000000;

int main(int argc, char **argv) {

  /*
   * MPI variables
   */
  int mpi_size, mpi_rank;
  MPI_Comm comm = MPI_COMM_WORLD;
  MPI_Info info = MPI_INFO_NULL;

  /*
   * Initialize MPI
   */
  MPI_Init(&argc, &argv);
  MPI_Comm_size(comm, &mpi_size);
  MPI_Comm_rank(comm, &mpi_rank);

  const size_t vector_dim = 10;
  const int rank_array = 1;

  std::vector<int> my_data(mpi_rank + vector_dim, 1 + mpi_rank);
  unsigned int my_size = my_data.size();
  std::cout << "\nvector done1";
  std::vector<unsigned int> rank_sizes(mpi_size);
  // Use all reduce
  MPI_Allgather(&my_size, 1, MPI_UNSIGNED, rank_sizes.data(), 1, MPI_UNSIGNED,
                comm);

  std::cout << "\nvector done2";
  unsigned int total_size = 0;
  for (const auto &i : rank_sizes)
    total_size += i;
  std::cout << "\nvector done3";
  unsigned int my_start = 0;
  for (unsigned int i = 0; i < mpi_rank; i++)
    my_start += rank_sizes[i];

  std::cout << "\nvector done";

  /*
   * HDF5 APIs definitions
   */
  hid_t file_id, dset_id;    /* file and dataset identifiers */
  hid_t filespace, memspace; /* file and memory dataspace identifiers */
  hsize_t dimsf[rank_array]; /* dataset dimensions */
  int *data;                 /* pointer to data buffer to write */
  hsize_t count[rank_array]; /* hyperslab selection parameters */
  hsize_t offset[rank_array];
  hid_t plist_id; /* property list identifier */
  int i;
  herr_t status;

  /*
   * Set up file access property list with parallel I/O access
   */
  plist_id = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(plist_id, comm, info);

  /*
   * Create a new file collectively and release property list identifier.
   */
  file_id = H5Fcreate(H5FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);
  H5Pclose(plist_id);

  /*
   * Create the dataspace for the dataset.
   */
  dimsf[0] = total_size;
  filespace = H5Screate_simple(rank_array, dimsf, NULL);

  /*
   * Create the dataset with default properties and close filespace.
   */
  dset_id = H5Dcreate(file_id, DATASETNAME, H5T_NATIVE_INT, filespace,
                      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Sclose(filespace);

  /*
   * Each process defines dataset in memory and writes it to the hyperslab
   * in the file.
   */
  count[0] = my_size;
  // count[1] = dimsf[1];
  offset[0] = my_start; // mpi_rank * count[0];
  // offset[1] = 0;
  memspace = H5Screate_simple(rank_array, count, NULL);

  /*
   * Select hyperslab in the file.
   */
  filespace = H5Dget_space(dset_id);
  H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, count, NULL);

  /*
   * Create property list for collective dataset write.
   */
  plist_id = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE);

  status = H5Dwrite(dset_id, H5T_NATIVE_INT, memspace, filespace, plist_id,
                    my_data.data());

  /*
   * Close/release resources.
   */
  H5Dclose(dset_id);
  H5Sclose(filespace);
  H5Sclose(memspace);
  H5Pclose(plist_id);
  H5Fclose(file_id);

  MPI_Finalize();

  return 0;
}
