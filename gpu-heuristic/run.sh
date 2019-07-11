#!/bin/bash

function EXEC {
	$*
	# if [ $? -ne 0 ]
	# then
	# 	#echo "Processo Interrompido"
	# 	#exit 1
	# fi
}

function CTRLC {
	echo "CTRL C ---------------------------------- PRESSIONADO!!!"
	exit 1 
}


execpath="./bin/HEA"
workflows="../input/gpu/"
dag=".dag"
declare -a wkf=("CyberShake_30_gpu" "CyberShake_50_gpu" "CyberShake_100_gpu" "Epigenomics_24_gpu" "Epigenomics_46_gpu" "Epigenomics_100_gpu" "Montage_25_gpu" "Montage_50_gpu" "Montage_100_gpu" "Inspiral_30_gpu" "Inspiral_50_gpu" "Inspiral_100_gpu" "Sipht_30_gpu" "Sipht_60_gpu" "Sipht_100_gpu")
trap CTRLC SIGINT
echo "instance,alpha,maxtime,maxcost,makespam,cost,FO"
## now loop through the above array
for (( a = 0; a <= 0; a++))
do
	# for file in "${wkf[@]}"
	# do
		for ((  i = 1 ;  i <= 1;  i++  )) 
		do	
			echo  -ne $file',' & EXEC $execpath -x 5 -a $a -s $i -w ../input/gpu/gpu.dag
			# EXEC $execpath -a $a -s $i -c $clusters -w $workflows$file$dag >> ./results/local_search_testing/"GRASP_new_LS.txt"
		done			
	# done
	# echo "Creating .csv ...\n" & EXEC ./results/script ./results/local_search_testing/"GRASP_new_LS.txt" ./results/local_search_testing/"GRASP_new_LS.csv" 10
done
