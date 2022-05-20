#clear-caches
export LD_LIBRARY_PATH=/usr/local/lib:/proj/tasrdma-PG0/hemem-defrag/src/:${LD_LIBRARY_PATH}
echo "=== Sparse set, 8 GB total data, 2 GB hot set, where 1 in 2 normal pages (1 GB) is hot ===" >> results.txt
./gups-pebs-sparse 16 100000000 33 8 30 2 >> results.txt
echo "=== Sparse set, 8 GB total data, 4 GB hot set, where 1 in 4 normal pages (1 GB) is hot ===" >> results.txt
./gups-pebs-sparse 16 100000000 33 8 30 4 >> results.txt
echo "=== Sparse set, 8 GB total data, 8 GB hot set, where 1 in 8 normal pages (1 GB) is hot ===" >> results.txt
./gups-pebs-sparse 16 100000000 33 8 30 8 >> results.txt
echo "=== Dense set, 8 GB total data, 1 GB hot set ===" >> results.txt
./gups-pebs 16 100000000 33 8 30 >> results.txt
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
