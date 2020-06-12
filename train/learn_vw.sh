jobs=16

neighbors=$1
population=$2

echo neighbors $neighbors population $population

train_len=`cat "../orig_data/training_sample_ids$data_suffix.list" | wc -l`

model_out_path=../models/vw/neighbors=$neighbors$population
mkdir -p $model_out_path

target_snp=`cat ../orig_data/target_geno_model_coordinates.txt`
# target_snp=`head -n 4 ../orig_data/target_geno_model_coordinates.txt`

mkdir -p tmp

function learn()
{
  pos=$1
  neighbors=$2
  train_len=$3
  model_out_path=$4
  population=$5

  data_out_path=./tmp

  echo SNP $pos

  # export data
  python3 to_vw.py -n $neighbors -s $pos -o $data_out_path --tag_file ../data/tag_train$population.pickle --target_file ../data/target_train$population.pickle
  rm $data_out_path/$pos.y

  file_X=$data_out_path/$pos.X

  for snp in 0 1 2
  do
    file_y=$data_out_path/$pos"_"$snp.y
    model=$model_out_path/$pos"_"$snp.model
    model_hr=${model/.model}.hr
    cache=$model.cache

    id="$pos"_"$snp"
    id+=":neighbors=$neighbors"
    id+=":train_samples=`paste -d ' ' $file_y $file_X | head -n $train_len | wc -l`"

    # params="-k "
    params="-k --quiet "
    params+="--loss_function logistic "
    params+="--holdout_off "
    params+="--passes 200 "
    params+="--id $id "

    # learn logreg model on first $train_len lines
    paste -d ' ' $file_y $file_X | head -n $train_len | vw $params -f $model --cache_file $cache

    # model to human-readable format
    paste -d ' ' $file_y $file_X | vw --quiet -i $model --invert_hash $model_hr -t

    rm $cache
    rm $model

    rm $file_y
  done

  rm $file_X
}
export -f learn

parallel -j $jobs learn {} $neighbors $train_len $model_out_path $population ::: $target_snp

rm -rf tmp
