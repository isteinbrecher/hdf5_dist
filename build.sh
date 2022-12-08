mpic++ -std=c++17 -O3 -o main.exe main.cpp -I /home/ivo/opt/spack/opt/spack/linux-ubuntu20.04-skylake/gcc-9.4.0/hdf5-1.12.2-2xgyynh7r6eqfsmdunkilg7c3olpt2k5/include -L /home/ivo/opt/spack/opt/spack/linux-ubuntu20.04-skylake/gcc-9.4.0/hdf5-1.12.2-2xgyynh7r6eqfsmdunkilg7c3olpt2k5/lib/ -lhdf5 -lhdf5_hl
mpirun -np 2 ./main.exe
h5dump SDS_row.h5