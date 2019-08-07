jobs=4

tag_type=10k
# tag_type=1k

data_out_path=data/vw/$tag_type/
mkdir -p $data_out_path

target_snp=`cat data/target_snp`
# target_snp=`head -n 2 data/target_snp`
# target_snp+=" 16367555"

function export_csv()
{
  snp_val=$1
  snp=${snp_val%_*}
  val=${snp_val##*_}
  tag_type=$2

  neighbors=20

  python3 stream_vw.py -n $neighbors --tag_file data/snp_tag_$tag_type.pickle -s $snp -v $val > data/vw/$tag_type/$snp_val.vw
}
export -f export_csv
parallel -j $jobs export_csv {1/.}_{2} $tag_type ::: $target_snp ::: 0 1 2

model_out_path=model/vw/$tag_type
mkdir -p $model_out_path

function learn()
{
  vw_data=$1
  model=$2

  # params="-k "
  params="-k --quiet "
  params+="--loss_function=logistic "
  params+="--holdout_off "
  params+="--passes 20 "
  # params+="-q :: --leave_duplicate_interactions " #quadratic interactions

  cat $vw_data | head -n 2250 | vw $params -f $model --cache_file $model.cache

  rm $model.cache
}
export -f learn

parallel -j $jobs learn $data_out_path/{1}_{2}.vw $model_out_path/{1}_{2}.model ::: $target_snp ::: 0 1 2


function test()
{
  snp=$1
  data_out_path=$2
  model_out_path=$3

  for val in 0 1 2
  do
    vw_data=$data_out_path/$snp"_"$val.vw
    model=$model_out_path/$snp"_"$val.model
    cat $vw_data | tail -n 254 | vw --quiet -i $model --leave_duplicate_interactions -t -r $model".tmp"
  done

  paste -d " " $model_out_path/$snp"_0.model.tmp" $model_out_path/$snp"_1.model.tmp" $model_out_path/$snp"_2.model.tmp"> $model_out_path/$snp.model.pred

  rm $model_out_path/$snp"_"*".model.tmp"

  val=`python3 roc_vw.py -s $snp -p $model_out_path/$snp.model.pred -i 2250`
  echo $snp","$val
}
export -f test
parallel -j $jobs test {1} $data_out_path $model_out_path ::: $target_snp

exit

# model to human-readable format
parallel -j $jobs vw --quiet -d {1} -i $model_out_path/{1}_{2}.model --invert_hash $model_out_path/{1}_{2}.model.hr -t ::: ::: $target_snp ::: 0 1 2


#remove data and models

rm $data_out_path -rf
rm $model_out_path -rf
