#clear-caches
# Export library
export LD_LIBRARY_PATH=/usr/local/lib:/proj/tasrdma-PG0/hemem-defrag/src/:${LD_LIBRARY_PATH}
# Allow a large number of remappings
echo 1000000 > /proc/sys/vm/max_map_count

thds=16
iters=1000000000
res_file=results4.txt
: '
rm ${res_file}


echo never > /sys/kernel/mm/transparent_hugepage/enabled
echo never > /sys/kernel/mm/transparent_hugepage/defrag

for sparse in 0 1 2 3; do 
	echo "<=== No THP, Dense set, 8 GB total data, 1 GB hot set, ${sparse} sparsity ===>" >> ${res_file}
	./gups-pebs-sparse ${thds} ${iters} 33 8 30 ${sparse} >> ${res_file}
done

echo always > /sys/kernel/mm/transparent_hugepage/enabled
echo always > /sys/kernel/mm/transparent_hugepage/defrag

for sparse in 0 1 2 3; do 
	echo "<=== THP, Dense set, 8 GB total data, 1 GB hot set, ${sparse} sparsity ===>" >> ${res_file}
	./gups-pebs-sparse ${thds} ${iters} 33 8 30 ${sparse} >> ${res_file}
done
'
echo never > /sys/kernel/mm/transparent_hugepage/enabled
echo never > /sys/kernel/mm/transparent_hugepage/defrag

for sparse in 0; do 
	echo "<=== Base pages, No THP, Dense set, 8 GB total data, 1 GB hot set, ${sparse} sparsity ===>" >> ${res_file}
	./gups-pebs-sparse-base ${thds} ${iters} 33 8 30 ${sparse} >> ${res_file}
done

echo always > /sys/kernel/mm/transparent_hugepage/enabled
echo always > /sys/kernel/mm/transparent_hugepage/defrag

for sparse in 0; do 
	echo "<=== Base pages, THP, Dense set, 8 GB total data, 1 GB hot set, ${sparse} sparsity ===>" >> ${res_file}
	./gups-pebs-sparse-base ${thds} ${iters} 33 8 30 ${sparse} >> ${res_file}
done
