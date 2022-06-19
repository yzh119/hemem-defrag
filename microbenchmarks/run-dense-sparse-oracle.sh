#!/bin/bash
#clear-caches
# Export library
export LD_LIBRARY_PATH=/usr/local/lib:/proj/tasrdma-PG0/hemem-defrag/src/:${LD_LIBRARY_PATH}
# Allow a large number of remappings
echo 10000000000 > /proc/sys/vm/max_map_count

thds=8
iters=1000000000
res_file=results/results-dense-sparse-oracle.txt
sparse_vals="0 1 2 3 4"
reps="1 2 3 4 5"

access_size=8
log_dataset=36
log_hotset=32


rm ${res_file}
: '
echo never > /sys/kernel/mm/transparent_hugepage/enabled
echo never > /sys/kernel/mm/transparent_hugepage/defrag
echo "<=== Oracle, No THP, first dense then sparse set then dense, 8 GB total data, 1 GB hot set ===>" >> ${res_file}
for sparse in 0 1 2 3; do 
	echo "<=== ${sparse} sparsity ===>" >> ${res_file}
	for iters2 in 0 1 2 3 4; do
		numactl --cpubind=0 ./gups-dense-sparse-oracle ${thds} ${iters} 33 8 30 ${sparse} results/oracle_tot_gups_huge_page_no_thp_${sparse}.txt >> ${res_file}
	done
done
'
echo always > /sys/kernel/mm/transparent_hugepage/enabled
echo always > /sys/kernel/mm/transparent_hugepage/defrag
echo "<=== Oracle, THP, first dense then sparse set then dense, $((1 << (${log_dataset} - 30) )) GB total data, $((1 << (${log_hotset} - 30) )) GB hot set ===>" >> ${res_file}
for sparse in ${sparse_vals}; do 
	echo "<=== ${sparse} sparsity ===>" >> ${res_file}
	for iters2 in ${reps}; do
		numactl --cpubind=0 ./gups-dense-sparse-oracle ${thds} ${iters} ${log_dataset} ${access_size} ${log_hotset} ${sparse} results/oracle_tot_gups_huge_page_thp_${sparse}.txt >> ${res_file}
	done
done
