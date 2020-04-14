mkdir -p logs

for nb_targets in 80882 20000 40000
do
    head -n $nb_targets ../../data/target_geno_model_coordinates.txt > targets_tmp.txt
    for suffix in "" "_AFR" "_AMR" "_EUR" 
    do     
        for n in 1 3 5 10 15 20 25 30 35 40 45 50
        do 
            sed -i "s/MODEL_FILE=.*/MODEL_FILE=\"..\/..\/ml\/model\/hr\/neighbors="$n"_scale=16384$suffix\"/" Makefile-final.inc  
            sed -i "s/TARGET_HEADERS=.*/TARGET_HEADERS=targets_tmp.txt/" Makefile-final.inc  
	
            make clean > /dev/null
            log_file=logs/$n$suffix"_"$nb_targets.log
            make suffix=$suffix  result.csv > $log_file

	    python3 ../parse_log.py $log_file $nb_targets,$suffix,$n     
        done
    done
done

rm targets_tmp.txt
