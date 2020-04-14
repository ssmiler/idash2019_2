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

idxFileA=../../data/testing_sample_ids.list
for nb_targets in 80882 20000 40000
do
    head -n $nb_targets ../../data/target_geno_model_coordinates.txt > targets_tmp.txt
    for suffix in "" "_AFR" "_AMR" "_EUR"
    do
        idxFileB=../../data/testing_sample_ids$suffix.list

        get_dataset $idxFileA $idxFileB ../../data/tag_testing.txt tag_testing$suffix.txt
        get_dataset $idxFileA $idxFileB ../../data/target_testing.txt target_testing$suffix.txt

        for n in 1 3 5 10 15 20 25 30 35 40 45 50
        do
            sed -i "s/MODEL_FILE=.*/MODEL_FILE=\"..\/..\/ml\/model\/hr\/neighbors="$n"_scale=16384$suffix\"/" Makefile-final.inc
            sed -i "s/TARGET_HEADERS=.*/TARGET_HEADERS=targets_tmp.txt/" Makefile-final.inc
            sed -i "s/CHALLENGE_FILE=.*/CHALLENGE_FILE=tag_testing$suffix.txt/" Makefile-final.inc
            sed -i "s/TARGET_FILE=.*/TARGET_FILE=target_testing$suffix.txt/" Makefile-final.inc

            make clean > /dev/null
            log_file=logs/$n$suffix"_"$nb_targets.log
            make result.csv > $log_file

            python3 ../parse_log.py $log_file $nb_targets,$suffix,$n
        done
    done
done

rm targets_tmp.txt
