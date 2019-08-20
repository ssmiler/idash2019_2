jobs=4

neighbors=$1
tag_type=$2

train_len=2250
if [ -n "$3" ]; then
  train_len=$3
  model_suffix=_final
fi

echo $tag_type $neighbors $train_len

data_out_path=/dev/shm/data/
mkdir -p $data_out_path

model_out_path=model/vw/$tag_type"_"neighbors=$neighbors$model_suffix/
mkdir -p $model_out_path

target_snp=`cat data/target_snp`
# target_snp=`head -n 2 data/target_snp`
# target_snp="18170885 18490550 18633682 18677838 18733610 18830098 19046187 20143073 20398042 20439521 20441632 20792342 21750234 21957238 22489212 24379740 24386330 24906710 186803226 195915482"

function export_csv()
{
  snp=$1
  tag_type=$2
  data_out_path=$3
  neighbors=$4

  python3 to_vw.py -n $neighbors --tag_file data/snp_tag_$tag_type.pickle -s $snp -o $data_out_path
}
export -f export_csv
parallel -j $jobs export_csv {1} $tag_type $data_out_path $neighbors ::: $target_snp

function learn()
{
  snp=$1
  val=$2
  data_out_path=$3
  model_out_path=$4
  train_len=$5

  file_y=$data_out_path/$snp"_"$val.y
  file_X=$data_out_path/$snp.X
  model=$model_out_path/$snp"_"$val.model
  cache=$model.cache

  id="$pos_snp:$tag_type"
  id+=":neighbors=$neighbors"
  id+=":train_samples=`paste -d ' ' $file_y $file_X | wc -l`"

  # params="-k "
  params="-k --quiet "
  params+="--loss_function=logistic "
  params+="--holdout_off "
  params+="--passes 200 "
  params+="--id $id "
  # params+="-q :: --leave_duplicate_interactions " #quadratic interactions

  paste -d ' ' $file_y $file_X | head -n $train_len | vw $params -f $model --cache_file $cache

  rm $cache
}
export -f learn
parallel -j $jobs learn {1} {2} $data_out_path $model_out_path $train_len ::: $target_snp ::: 0 1 2

# model to human-readable format
function to_hr()
{
  snp=$1
  val=$2
  data_out_path=$3
  model_out_path=$4

  model=$model_out_path/$snp"_"$val.model
  file_y=$data_out_path/$snp"_"$val.y
  file_X=$data_out_path/$snp.X

  paste -d ' ' $file_y $file_X | vw --quiet -i $model --invert_hash $model.hr -t --leave_duplicate_interactions
}
export -f to_hr
parallel -j $jobs to_hr {1} {2} $data_out_path $model_out_path ::: $target_snp ::: 0 1 2

exit

#remove data and models
rm $data_out_path -rf
rm $model_out_path -rf




