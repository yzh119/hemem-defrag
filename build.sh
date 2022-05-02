#!/bin/bash
# install dependencies
sudo apt install clang pkg-config libcapstone-dev 

# build hoard
cd Hoard/src
make
cd ../../

# build syscall_intercept
cd syscall_intercept
mkdir -p build
cd build
cmake ..
make -j
cd ../../
