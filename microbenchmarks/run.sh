#clear-caches
export LD_LIBRARY_PATH=/usr/local/lib:/proj/tasrdma-PG0/hemem-defrag/src/:${LD_LIBRARY_PATH}
echo "=== 32 ===" >> results.txt
./gups-pebs 16 1000000 33 8 32 >> results.txt
# #clear-caches
# echo "=== 31 ===" >> results.txt
# numactl -N0 -m0 -- ./gups-pebs 16 1000000000 39 8 31 >> results.txt
# #clear-caches
# echo "=== 32 ===" >> results.txt
# numactl -N0 -m0 -- ./gups-pebs 16 1000000000 39 8 32 >> results.txt
# #clear-caches
# echo "=== 33 ===" >> results.txt
# numactl -N0 -m0 -- ./gups-pebs 16 1000000000 39 8 33 >> results.txt
# #clear-caches
# echo "=== 34 ===" >> results.txt
# numactl -N0 -m0 -- ./gups-pebs 16 1000000000 39 8 34 >> results.txt
# #clear-caches
# echo "=== 35 ===" >> results.txt
# numactl -N0 -m0 -- ./gups-pebs 16 1000000000 39 8 35 >> results.txt
# #clear-caches
# echo "=== 36 ===" >> results.txt
# numactl -N0 -m0 -- ./gups-pebs 16 1000000000 39 8 36 >> results.txt
# #clear-caches
# echo "=== 37 ===" >> results.txt
# numactl -N0 -m0 -- ./gups-pebs 16 1000000000 39 8 37 >> results.txt
# #clear-caches
# echo "=== 38 ===" >> results.txt
# numactl -N0 -m0 -- ./gups-pebs 16 1000000000 39 8 38 >> results.txt
