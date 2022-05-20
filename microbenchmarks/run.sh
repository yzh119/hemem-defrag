#clear-caches
export LD_LIBRARY_PATH=/usr/local/lib:/proj/tasrdma-PG0/hemem-defrag/src/:${LD_LIBRARY_PATH}

rm results.txt

thds=16
iters=500000000

echo never > /sys/kernel/mm/transparent_hugepage/enabled
echo never > /sys/kernel/mm/transparent_hugepage/defrag
echo "<=== No THP, Dense set, 8 GB total data, 1 GB hot set ===>" >> results.txt
./gups-pebs ${thds} ${iters} 33 8 30 >> results.txt
echo "<=== No THP, Sparse set, 8 GB total data, 2 GB hot set, where 1 in 2 normal pages (1 GB) is hot ===>" >> results.txt
./gups-pebs-sparse ${thds} ${iters} 33 8 30 2 >> results.txt
echo "<=== No THP, Sparse set, 8 GB total data, 4 GB hot set, where 1 in 4 normal pages (1 GB) is hot ===>" >> results.txt
./gups-pebs-sparse ${thds} ${iters} 33 8 30 4 >> results.txt
echo "<=== No THP, Sparse set, 8 GB total data, 8 GB hot set, where 1 in 8 normal pages (1 GB) is hot ===>" >> results.txt
./gups-pebs-sparse ${thds} ${iters} 33 8 30 8 >> results.txt

echo always > /sys/kernel/mm/transparent_hugepage/enabled
echo always > /sys/kernel/mm/transparent_hugepage/defrag
echo "<=== THP, Dense set, 8 GB total data, 1 GB hot set ===>" >> results.txt
./gups-pebs ${thds} ${iters} 33 8 30 >> results.txt
echo "<=== THP, Sparse set, 8 GB total data, 2 GB hot set, where 1 in 2 normal pages (1 GB) is hot ===>" >> results.txt
./gups-pebs-sparse ${thds} ${iters} 33 8 30 2 >> results.txt
echo "<=== THP, Sparse set, 8 GB total data, 4 GB hot set, where 1 in 4 normal pages (1 GB) is hot ===>" >> results.txt
./gups-pebs-sparse ${thds} ${iters} 33 8 30 4 >> results.txt
echo "<=== THP, Sparse set, 8 GB total data, 8 GB hot set, where 1 in 8 normal pages (1 GB) is hot ===>" >> results.txt
./gups-pebs-sparse ${thds} ${iters} 33 8 30 8 >> results.txt

echo never > /sys/kernel/mm/transparent_hugepage/enabled
echo never > /sys/kernel/mm/transparent_hugepage/defrag
echo "<=== Base pages, No THP, Dense set, 8 GB total data, 1 GB hot set ===>" >> results.txt
./gups-pebs-base ${thds} ${iters} 33 8 30 >> results.txt
echo "<=== Base pages, No THP, Sparse set, 8 GB total data, 2 GB hot set, where 1 in 2 normal pages (1 GB) is hot ===>" >> results.txt
./gups-pebs-sparse-base ${thds} ${iters} 33 8 30 2 >> results.txt
echo "<=== Base pages, No THP, Sparse set, 8 GB total data, 4 GB hot set, where 1 in 4 normal pages (1 GB) is hot ===>" >> results.txt
./gups-pebs-sparse-base ${thds} ${iters} 33 8 30 4 >> results.txt
echo "<=== Base pages, No THP, Sparse set, 8 GB total data, 8 GB hot set, where 1 in 8 normal pages (1 GB) is hot ===>" >> results.txt
./gups-pebs-sparse-base ${thds} ${iters} 33 8 30 8 >> results.txt

echo always > /sys/kernel/mm/transparent_hugepage/enabled
echo always > /sys/kernel/mm/transparent_hugepage/defrag
echo "<=== Base pages, THP, Dense set, 8 GB total data, 1 GB hot set ===>" >> results.txt
./gups-pebs-base ${thds} ${iters} 33 8 30 >> results.txt
echo "<=== Base pages, THP, Sparse set, 8 GB total data, 2 GB hot set, where 1 in 2 normal pages (1 GB) is hot ===>" >> results.txt
./gups-pebs-sparse-base ${thds} ${iters} 33 8 30 2 >> results.txt
echo "<=== Base pages, THP, Sparse set, 8 GB total data, 4 GB hot set, where 1 in 4 normal pages (1 GB) is hot ===>" >> results.txt
./gups-pebs-sparse-base ${thds} ${iters} 33 8 30 4 >> results.txt
echo "<=== Base pages, THP, Sparse set, 8 GB total data, 8 GB hot set, where 1 in 8 normal pages (1 GB) is hot ===>" >> results.txt
./gups-pebs-sparse-base ${thds} ${iters} 33 8 30 8 >> results.txt
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
