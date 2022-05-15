#!/bin/bash
# install dependencies
sudo apt install clang pkg-config libcapstone-dev cmake

# build hoard
cd Hoard/src
make
sudo make install
cd ../../

# build syscall_intercept
cd syscall_intercept
mkdir -p build
cd build
cmake ..
make -j
sudo make install
cd ../../

# build hemem
cd src
make -j
cd ../

# build microbenchmark
cd microbenchmark
make -j
cd ../
