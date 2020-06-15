# execute in run folder
cd `dirname $0`/run

mkdir -p logs

get_dataset() {
    idxFileA=$1
    idxFileB=$2
    inpFile=$3
    outFile=$4

    code="""
la = [1,2,3,4] + open('$idxFileA').readlines();
lb = [1,2,3,4] + open('$idxFileB').readlines();
import pandas as pd;
df = pd.read_csv('$inpFile', header=None, sep='\t');
assert(df.shape[1] == len(la));
df.columns = la;
df.loc[:, lb].to_csv('$outFile', index=False, header=False, sep='\t')
    """
    python3 -c "$code"
}
export -f get_dataset

idxFileA=../../orig_data/testing_sample_ids.list

for nb_cores in 1 4 8
    do

    sed -i "s/#define NB_THREADS.*/#define NB_THREADS $nb_cores/" ../idash.h
    make build 1> /dev/stderr 2>&1

    for nb_targets in 80882 20000 40000
    do
        head -n $nb_targets ../../orig_data/target_geno_model_coordinates.txt > targets_tmp.txt
        for population in "" "_AFR" "_AMR" "_EUR"
        do
            idxFileB=../../orig_data/testing_sample_ids$population.list

            get_dataset $idxFileA $idxFileB ../../orig_data/tag_testing.txt tag_testing$population.txt
            get_dataset $idxFileA $idxFileB ../../orig_data/target_testing.txt target_testing$population.txt

            for n in 51 10 15 20 25 30 35 40 45 50
            do
                sed -i "s/MODEL_FILE=.*/MODEL_FILE=\"..\/..\/models\/hr\/neighbors=$n$population\"/" Makefile-final.inc
                sed -i "s/TARGET_HEADERS=.*/TARGET_HEADERS=targets_tmp.txt/" Makefile-final.inc
                sed -i "s/CHALLENGE_FILE=.*/CHALLENGE_FILE=tag_testing$population.txt/" Makefile-final.inc
                sed -i "s/TARGET_FILE=.*/TARGET_FILE=target_testing$population.txt/" Makefile-final.inc

                exit

                make clean > /dev/null
                log_file=logs/$nb_cores"_"$nb_targets$population"_"$n.log
                make auc > $log_file

                python3 ../parse_log.py $log_file $nb_cores,$nb_targets,$population,$n
            done

            rm tag_testing$population.txt 1> /dev/stderr 2>&1
            rm target_testing$population.txt 1> /dev/stderr 2>&1
        done

        rm targets_tmp.txt 1> /dev/stderr 2>&1
    done
done
