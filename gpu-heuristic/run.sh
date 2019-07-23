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
declare -a wkf=("gpu" "CyberShake_30_gpu" "CyberShake_50_gpu" "CyberShake_100_gpu" "Epigenomics_24_gpu" "Epigenomics_46_gpu" "Epigenomics_100_gpu" "Montage_25_gpu" "Montage_50_gpu" "Montage_100_gpu" "Inspiral_30_gpu" "Inspiral_50_gpu" "Inspiral_100_gpu" "Sipht_30_gpu" "Sipht_60_gpu" "Sipht_100_gpu")
# declare -a wkf=("gpu" "CyberShake_30_gpu" "CyberShake_50_gpu" "CyberShake_100_gpu")
trap CTRLC SIGINT
# echo "instance,alpha,maxtime,maxcost,makespam,cost,FO"
## now loop through the above array
for (( a = 2; a <= 2; a++))
do
	for file in "${wkf[@]}"
	do
		if [ $file == "gpu" ] || [ $file == "CyberShake_30_gpu" ] || [ $file == "CyberShake_50_gpu" ] || [ $file == "CyberShake_100_gpu" ] || [ $file == "Montage_25_gpu" ] || [ $file == "Montage_100_gpu" ] || [ $file == "Inspiral_30_gpu" ];
		then
			i=1
		fi

		if [ $file == "Epigenomics_46_gpu" ];
		then
			i=5
		fi
		if [ $file == "Epigenomics_100_gpu" ];
		then
			i=2
		fi
		if [ $file == "Montage_50_gpu" ] || [ $file == "Sipht_30_gpu" ] || [ $file == "Epigenomics_24_gpu" ];
		then
			i=8
		fi
		if [ $file == "Inspiral_50_gpu" ];
		then
			i=3
		fi
		if [ $file == "Inspiral_100_gpu" ] || [ $file == "Sipht_60_gpu" ];
		then
			i=7
		fi
		if [ $file == "Sipht_100_gpu" ];
		then
			i=6
		fi

		# echo $i
		# for ((  i =  1;  i <= 10;  i++  )) 
		# do	
			echo  -ne $file '\n' >> ../BEST_SOLUTIONS.txt & EXEC $execpath -x 5 -a $a -s $i -w $workflows$file$dag >> ../BEST_SOLUTIONS.txt
			# echo  -ne $file '' >> ../results/final_exec.txt & EXEC $execpath -x 5 -a $a -s $i -w $workflows$file$dag >> ../results/final_exec.txt
			# EXEC $execpath -a $a -s $i -c $clusters -w $workflows$file$dag >> ./results/local_search_testing/"GRASP_new_LS.txt"
		# done			
	done
	# echo "Creating .csv ...\n" & EXEC ./results/script ./results/local_search_testing/"GRASP_new_LS.txt" ./results/local_search_testing/"GRASP_new_LS.csv" 10
done
