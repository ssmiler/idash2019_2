mkdir -p logs

for suffix in "" "_AFR" "_AMR" "_EUR" 
do     
    for n in 1 3 5 10 15 20 25 30 35 40 45 50
    do
        sed -i "s/MODEL_FILE=.*/MODEL_FILE=\"..\/..\/ml\/model\/hr\/neighbors="$n"_scale=16384$suffix\"/" Makefile-final.inc  

	make clean > /dev/null
        log_file=logs/$n$suffix.log
        make result.csv > $log_file

	python3 ../parse_log.py $log_file $suffix,$n     
    done
done

