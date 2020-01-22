jobs=4

neighbors=$1

train_len=1200

if [ -n "$2" ]
then
  train_len=$2
  model_suffix=_final
fi

echo $neighbors $train_len $model_suffix

model_out_path=model/vw/neighbors=$neighbors$model_suffix
mkdir -p $model_out_path

target_snp=`cat data/target_snp`
# target_snp=`head -n 2 data/target_snp`
# target_snp="16051248"

function learn()
{
  pos=$1
  neighbors=$2
  train_len=$3
  model_out_path=$4

  data_out_path=/dev/shm

  echo $pos $neighbors $model_out_path $train_len

  # export data
  python3 to_vw.py -n $neighbors -s $pos -o $data_out_path --tag_file data/tag_training.pickle --target_file data/target_training.pickle
  rm $data_out_path/$pos.y

  file_X=$data_out_path/$pos.X

  for snp in 0 1 2
  do
    file_y=$data_out_path/$pos"_"$snp.y
    model=$model_out_path/$pos"_"$snp.model
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
    # params+="-q :: --leave_duplicate_interactions " #quadratic interactions

    declare -A opt_params=(
      ["orig"]=""
      # ["bs=5"]="--bootstrap 5"
      # ["bs=5_rw"]="--bootstrap 5 --random_weights 1"
      # ["bs=10"]="--bootstrap 10"
      # ["bs=10_rw"]="--bootstrap 10 --random_weights 1"
      # ["ftrl"]="--ftrl"
      # ["ftrl_bs=10_rw"]="--ftrl --bootstrap 10 --random_weights 1"
      # ["boost=5_alg=BBM"]="--boosting 5 --alg BBM"
      # ["boost=5_alg=log"]="--boosting 5 --alg logistic"
    )
    for fp in "${!opt_params[@]}"
    do
      opt_param="${opt_params[$fp]}"
      paste -d ' ' $file_y $file_X | head -n $train_len | vw $params $opt_param -f $model --cache_file $cache

      # model to human-readable format
      paste -d ' ' $file_y $file_X | vw --quiet -i $model --invert_hash ${model/.model}_$fp.hr -t

      rm $cache
      rm $model
    done

    rm $file_y
  done

  rm $file_X
}
export -f learn

parallel -j $jobs learn {} $neighbors $train_len $model_out_path ::: $target_snp

exit

#remove data and models
rm $model_out_path -rf




