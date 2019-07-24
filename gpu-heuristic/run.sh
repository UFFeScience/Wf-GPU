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
declare -a wkf=("3_toy_15_A" "3_toy_15_B" "3_toy_15_C" "5_toy_15_A" "5_toy_15_B" "5_toy_15_C" "3_toy_10_A" "3_toy_10_B" "3_toy_10_C" "5_toy_10_A" "5_toy_10_B" "5_toy_10_C" "3_toy_5_A" "3_toy_5_C" "5_toy_5_A" "5_toy_5_B" "5_toy_5_C" "gpu" "CyberShake_30_gpu" "CyberShake_50_gpu" "CyberShake_100_gpu" "Epigenomics_24_gpu" "Epigenomics_46_gpu" "Epigenomics_100_gpu" "Montage_25_gpu" "Montage_50_gpu" "Montage_100_gpu" "Inspiral_30_gpu" "Inspiral_50_gpu" "Inspiral_100_gpu" "Sipht_30_gpu" "Sipht_60_gpu" "Sipht_100_gpu")
# declare -a wkf=("5_toy_15_A" "5_toy_15_B" "5_toy_15_C" "3_toy_10_A" "3_toy_10_B" "3_toy_10_C" "5_toy_10_A" "5_toy_10_B" "5_toy_10_C" "3_toy_5_A" "3_toy_5_C" "5_toy_5_A" "5_toy_5_B" "5_toy_5_C")
# declare -a wkf=("3_toy_15_B")
# declare -a wkf=("gpu")
trap CTRLC SIGINT
echo "instance,alpha,maxtime,maxcost,makespam,cost,FO" >> ../results/second_run.csv
## now loop through the above array
for (( a = 0; a <= 0; a++))
do
	for file in "${wkf[@]}"
	do
		# if [ $file == "gpu" ] || [ $file == "CyberShake_30_gpu" ] || [ $file == "CyberShake_50_gpu" ] || [ $file == "CyberShake_100_gpu" ] || [ $file == "Montage_25_gpu" ] || [ $file == "Montage_100_gpu" ] || [ $file == "Inspiral_30_gpu" ];
		# then
		# 	i=1
		# fi

		# if [ $file == "Epigenomics_46_gpu" ];
		# then
		# 	i=5
		# fi
		# if [ $file == "Epigenomics_100_gpu" ];
		# then
		# 	i=2
		# fi
		# if [ $file == "Montage_50_gpu" ] || [ $file == "Sipht_30_gpu" ] || [ $file == "Epigenomics_24_gpu" ];
		# then
		# 	i=8
		# fi
		# if [ $file == "Inspiral_50_gpu" ];
		# then
		# 	i=3
		# fi
		# if [ $file == "Inspiral_100_gpu" ] || [ $file == "Sipht_60_gpu" ];
		# then
		# 	i=7
		# fi
		# if [ $file == "Sipht_100_gpu" ];
		# then
		# 	i=6
		# fi

		# echo $i
		for ((  i =  10;  i <= 10;  i++  )) 
		do
			# echo  -ne $file '' & EXEC $execpath -x 5 -a $a -s $i -w $workflows$file$dag	
			# echo  -ne $file '\n' >> ../BEST_SOLUTIONS.txt & EXEC $execpath -x 5 -a $a -s $i -w $workflows$file$dag >> ../BEST_SOLUTIONS.txt
			echo  -ne $file ',' >> ../results/second_run.csv & EXEC $execpath -x 5 -a $a -s $i -w $workflows$file$dag >> ../results/second_run.csv
			# EXEC $execpath -a $a -s $i -c $clusters -w $workflows$file$dag >> ./results/local_search_testing/"GRASP_new_LS.txt"
		done			
	done
	# echo "Creating .csv ...\n" & EXEC ./results/script ./results/local_search_testing/"GRASP_new_LS.txt" ./results/local_search_testing/"GRASP_new_LS.csv" 10
done
