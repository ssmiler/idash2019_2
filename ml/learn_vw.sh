jobs=4

tag_type=10k
# tag_type=1k

data_out_path=/dev/shm/data/vw/$tag_type/
mkdir -p $data_out_path

target_snp=`cat data/target_snp`
# target_snp=`head -n 2 data/target_snp`
# target_snp="18170885 18490550 18633682 18677838 18733610 18830098 19046187 20143073 20398042 20439521 20441632 20792342 21750234 21957238 22489212 24379740 24386330 24906710 186803226 195915482"

function export_csv()
{
  snp=$1
  tag_type=$2
  data_out_path=$3

  neighbors=20

  python3 to_vw.py -n $neighbors --tag_file data/snp_tag_$tag_type.pickle -s $snp -o $data_out_path
}
export -f export_csv
parallel -j $jobs export_csv {1} $tag_type $data_out_path ::: $target_snp

model_out_path=model/vw/$tag_type
mkdir -p $model_out_path

function learn()
{
  snp=$1
  val=$2
  data_out_path=$3
  model_out_path=$4

  params="-k "
  # params="-k --quiet "
  params+="--loss_function=logistic "
  params+="--holdout_off "
  params+="--passes 100 "
  # params+="-q :: --leave_duplicate_interactions " #quadratic interactions

  file_y=$data_out_path/$snp"_"$val.y
  file_X=$data_out_path/$snp.X
  model=$model_out_path/$snp"_"$val.model
  cache=$model.cache

  paste -d ' ' $file_y $file_X | head -n 2250 | vw $params -f $model --cache_file $cache

  rm $cache
}
export -f learn
parallel -j $jobs learn {1} {2} $data_out_path $model_out_path ::: $target_snp ::: 0 1 2


function test()
{
  snp=$1
  data_out_path=$2
  model_out_path=$3

  for val in 0 1 2
  do
    file_y=$data_out_path/$snp"_"$val.y
    file_X=$data_out_path/$snp.X
    model=$model_out_path/$snp"_"$val.model
    # paste -d ' ' $file_y $file_X | head -n 2250 | vw --quiet -i $model --leave_duplicate_interactions -t -r $model.tmp
    paste -d ' ' $file_y $file_X | tail -n 254 | vw --quiet -i $model --leave_duplicate_interactions -t -r $model.tmp
  done

  paste -d " " $model_out_path/$snp"_0.model.tmp" $model_out_path/$snp"_1.model.tmp" $model_out_path/$snp"_2.model.tmp"> $model_out_path/$snp.model.pred

  rm $model_out_path/$snp"_"*".model.tmp"

  val=`python3 roc_vw.py -s $snp -p $model_out_path/$snp.model.pred -i 2250`
  echo $snp","$val
}
export -f test

echo "snp,auc_0,auc_1,auc_2,auc"
parallel -j $jobs -k test {1} $data_out_path $model_out_path ::: $target_snp

exit

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


#remove data and models

rm $data_out_path -rf
rm $model_out_path -rf





function test_step_model()
{
  snp=$1
  data_out_path=$2
  model_out_path=$3

  for val in 0 1 2
  do
    file_y=$data_out_path/$snp"_"$val.y
    file_X=$data_out_path/$snp.X
    for model in $model_out_path/$snp"_"$val.model.*
    do
      paste -d ' ' $file_y $file_X | tail -n 254 | vw --quiet -i $model --leave_duplicate_interactions -t -r $model.tmp
    done
  done

  for model in $model_out_path/$snp"_"*.model.? $model_out_path/$snp"_"*.model.??
  do
    step=${model##*.}
    paste -d " " $model_out_path/$snp"_0.model.$step.tmp" $model_out_path/$snp"_1.model.$step.tmp" $model_out_path/$snp"_2.model.$step.tmp"> $model_out_path/$snp.model.$step.pred

    val=`python3 roc_vw.py -s $snp -p $model_out_path/$snp.model.$step.pred -i 2250`
    echo $snp,$step,$val
  done

  rm $model_out_path/$snp"_"*".model."*".tmp"
  # rm $model_out_path/$snp"_"*".model."*".pred"
}
export -f test_step_model

echo "snp,step,auc_0,auc_1,auc_2,auc"
parallel -j $jobs test_step_model {1} $data_out_path $model_out_path ::: $target_snp
