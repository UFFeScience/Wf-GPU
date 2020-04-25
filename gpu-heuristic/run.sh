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
#declare -a wkf=("3_toy_15_A" "3_toy_15_B" "3_toy_15_C" "5_toy_15_A" "5_toy_15_B" "5_toy_15_C" "3_toy_10_A" "3_toy_10_B" "3_toy_10_C" "5_toy_10_A" "5_toy_10_B" "5_toy_10_C" "3_toy_5_A" "3_toy_5_C" "5_toy_5_A" "5_toy_5_B" "5_toy_5_C" "gpu" "CyberShake_30_gpu" "CyberShake_50_gpu" "CyberShake_100_gpu" "Epigenomics_24_gpu" "Epigenomics_46_gpu" "Epigenomics_100_gpu" "Montage_25_gpu" "Montage_50_gpu" "Montage_100_gpu" "Inspiral_30_gpu" "Inspiral_50_gpu" "Inspiral_100_gpu" "Sipht_30_gpu" "Sipht_60_gpu" "Sipht_100_gpu")
#declare -a maxcosts=(154	54	136	88	46	56	168	198	440	268	300	450	120	80	120	108	96	34	282	1758	5353	9888	3377	197180	454	138	1792	772	2044	3008 669 4422 2256 )
#declare -a maxtimes=(36	52	54	38	44	54	46	16	42	82	22	68	10	6	10	16	14	32	2820	2332	4604	6788	21252	106878	340	816	1018	2456	3006	6140	5888	8774	15160 )

declare -a wkf=("3_toy_10_A" "3_toy_10_B" "3_toy_10_C" "3_toy_15_A" "3_toy_15_B" "3_toy_15_C" "3_toy_5_A" "3_toy_5_B" "3_toy_5_C" "5_toy_10_A" "5_toy_10_B" "5_toy_10_C" "5_toy_15_A" "5_toy_15_B" "5_toy_15_C" "5_toy_5_A" "5_toy_5_B" "5_toy_5_C")

declare -a maxcosts=(450 380 720 200 190 240 120 230 400 560 488 960 176 160 200 120 230 400)
declare -a maxtimes=(46 39 73 21 20 25 13 24 41 71 62 121 23 21 26 13 24 41)

 declare -a wkf=("3_toy_5_A" "3_toy_10_A" "3_toy_10_B" "3_toy_5_C" "3_toy_5_B" "3_toy_10_C" "3_toy_15_A" "3_toy_15_B" "3_toy_15_C")
  declare -a wkf=("3_toy_10_A")
# declare -a wkf=("5_toy_15_A" "5_toy_15_B" "5_toy_15_C" "3_toy_10_A" "3_toy_10_B" "3_toy_10_C" "5_toy_10_A" "5_toy_10_B" "5_toy_10_C" "3_toy_5_A" "3_toy_5_C" "5_toy_5_A" "5_toy_5_B" "5_toy_5_C")
#declare -a wkf=("Montage_25_gpu" "Montage_50_gpu" "Montage_100_gpu" "Inspiral_30_gpu" "Inspiral_50_gpu" "Inspiral_100_gpu" "Sipht_30_gpu" "Sipht_60_gpu" "Sipht_100_gpu")


declare -a maxcosts=(120 450 380 400 230 720 200 190 240)
declare -a maxtimes=(13 46 39 41 24 73 21 20 25)

declare -a maxcosts=(450)
declare -a maxtimes=(46)
# declare -a wkf=("gpu")
trap CTRLC SIGINT
# echo "instance,alpha,maxtime,maxcost,makespam,cost,FO" >> ../results/second_run.csv
## now loop through the above array
for (( a = 2; a <= 2; a++))
do
	for(( f = 0; f < ${#wkf[@]} ; f=$f+1 ))
	# for file in "${wkf[@]}"
	do
		file=${wkf[${f}]}
		maxcost=${maxcosts[${f}]}
		maxtime=${maxtimes[${f}]}
		# echo $file " " $maxcost " " $maxtime

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
		for ((  i =  1;  i <= 1;  i++  )) 
		do
			echo  -ne $file '' & EXEC $execpath -t "$maxtime" -c "$maxcost" -x 5 -a $a -s $i -w $workflows$file$dag 
			# echo  -ne $file '\n' >> ../BEST_SOLUTIONS.txt & EXEC $execpath -x 5 -a $a -s $i -w $workflows$file$dag >> ../BEST_SOLUTIONS.txt
			# echo  -ne $file '' >> ../results/sciphy.txt & EXEC $execpath -t "$maxtime" -c "$maxcost" -x 5 -a $a -s $i -w $workflows$file$dag >> ../results/sciphy.txt
			# EXEC $execpath -a $a -s $i -c $clusters -w $workflows$file$dag >> ./results/local_search_testing/"GRASP_new_LS.txt"
		done			
	done
	# echo "Creating .csv ...\n" & EXEC ./results/script ./results/local_search_testing/"GRASP_new_LS.txt" ./results/local_search_testing/"GRASP_new_LS.csv" 10
done
