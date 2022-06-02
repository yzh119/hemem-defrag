#clear-caches
# Export library
export LD_LIBRARY_PATH=/usr/local/lib:/proj/tasrdma-PG0/hemem-defrag/src/:${LD_LIBRARY_PATH}
# Allow a large number of remappings
echo 10000000 > /proc/sys/vm/max_map_count

thds=16
iters=1000000000
res_file=results/results-sparse-oracle.txt

rm ${res_file}

echo never > /sys/kernel/mm/transparent_hugepage/enabled
echo never > /sys/kernel/mm/transparent_hugepage/defrag

for sparse in 0 1 2 3; do 
	echo "<=== Huge pages, No THP, Dense set, 8 GB total data, 1 GB hot set, ${sparse} sparsity ===>" >> ${res_file}
	for iters2 in 0 1 2 3 4; do
		./gups-sparse-oracle ${thds} ${iters} 33 8 30 ${sparse} >> ${res_file}
	done
done

for sparse in 0 1 2 3; do 
	echo "<=== Base pages, No THP, Dense set, 8 GB total data, 1 GB hot set, ${sparse} sparsity ===>" >> ${res_file}
	for iters2 in 0 1 2 3 4; do
		./gups-sparse-oracle-base ${thds} ${iters} 33 8 30 ${sparse} >> ${res_file}
	done
done

echo always > /sys/kernel/mm/transparent_hugepage/enabled
echo always > /sys/kernel/mm/transparent_hugepage/defrag

for sparse in 0 1 2 3; do 
	echo "<=== Huge pages, THP, Dense set, 8 GB total data, 1 GB hot set, ${sparse} sparsity ===>" >> ${res_file}
	for iters2 in 0 1 2 3 4; do
		./gups-sparse-oracle ${thds} ${iters} 33 8 30 ${sparse} >> ${res_file}
	done
done

for sparse in 0 1 2 3; do 
	echo "<=== Base pages, THP, Dense set, 8 GB total data, 1 GB hot set, ${sparse} sparsity ===>" >> ${res_file}
	for iters2 in 0 1 2 3 4; do
		./gups-sparse-oracle-base ${thds} ${iters} 33 8 30 ${sparse} >> ${res_file}
	done
done
