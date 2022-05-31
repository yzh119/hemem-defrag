#clear-caches
# Export library
export LD_LIBRARY_PATH=/usr/local/lib:/proj/tasrdma-PG0/hemem-defrag/src/:${LD_LIBRARY_PATH}
# Allow a large number of remappings
echo 1000000 > /proc/sys/vm/max_map_count

thds=16
iters=100000000
res_file=results-dense-sparse.txt

rm ${res_file}

echo never > /sys/kernel/mm/transparent_hugepage/enabled
echo never > /sys/kernel/mm/transparent_hugepage/defrag
echo "<=== Huge pages, No THP, first dense then sparse set then dense, 8 GB total data, 1 GB hot set ===>" >> ${res_file}
for sparse in 0 1 2 3; do 
	echo "<=== ${sparse} sparsity ===>" >> ${res_file}
	./gups-pebs-dense-sparse ${thds} ${iters} 33 8 30 ${sparse} >> ${res_file}
done

echo "<=== Base pages, No THP, first dense then sparse set then dense, 8 GB total data, 1 GB hot set ===>" >> ${res_file}
for sparse in 0 1 2 3; do 
	echo "<=== ${sparse} sparsity ===>" >> ${res_file}
	./gups-pebs-dense-sparse-base ${thds} ${iters} 33 8 30 ${sparse} >> ${res_file}
done

echo always > /sys/kernel/mm/transparent_hugepage/enabled
echo always > /sys/kernel/mm/transparent_hugepage/defrag
echo "<=== Huge pages, THP, first dense then sparse set then dense, 8 GB total data, 1 GB hot set ===>" >> ${res_file}
for sparse in 0 1 2 3; do 
	echo "<=== ${sparse} sparsity ===>" >> ${res_file}
	./gups-pebs-dense-sparse ${thds} ${iters} 33 8 30 ${sparse} >> ${res_file}
done

echo "<=== Base pages, THP, first dense then sparse set then dense, 8 GB total data, 1 GB hot set ===>" >> ${res_file}
for sparse in 0 1 2 3; do 
	echo "<=== ${sparse} sparsity ===>" >> ${res_file}
	./gups-pebs-dense-sparse-base ${thds} ${iters} 33 8 30 ${sparse} >> ${res_file}
done


