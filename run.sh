# run microbenchmark
export LD_LIBRARY_PATH=/usr/local/lib:${LD_LIBRARY_PATH}
LD_PRELOAD=./src/libhemem.so sh microbenchmark/run.sh
