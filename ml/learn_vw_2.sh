jobs=4

tag_type=10k
# tag_type=1k

model_out_path=model/vw/$tag_type/
mkdir -p $model_out_path

input_files=data/vw/$tag_type/16367555.vw
# input_files=data/vw/$tag_type/*.vw


function export_csv()
{
  snp=$1
  tag_type=$2

  python3 stream_vw.py -n 20 --tag_file data/snp_tag_$tag_type.pickle -s $snp > data/vw/$tag_type/$snp.vw
}
export -f export_csv
parallel -j $jobs export_csv {1/.} $tag_type ::: $input_files


function learn()
{
  vw_data=$1
  model=$2

  # params="-k "
  params="-k --quiet "
  params+="--loss_function=logistic "
  params+="--oaa 3 "
  params+="--holdout_off "
  params+="--passes 10 -c "
  # params+="-q :: --leave_duplicate_interactions " #quadratic interactions

  cat $vw_data | head -n 2250 | vw $params -f $model
}
export -f learn
parallel -j $jobs learn data/vw/$tag_type/{1/.}.vw $model_out_path/{1/.}.model ::: $input_files


function test()
{
  vw_data=$1
  model=$2
  snp=${vw_data%*.vw}
  snp=${snp##*/}

  # make predictions with each model
  cat $vw_data | tail -n 254 | vw --quiet -i $model --leave_duplicate_interactions -t  -r $model".pred"
  val=`python3 roc_vw.py -s $snp -p $model".pred" -i 2250`
  echo $snp","$val
}
export -f test
parallel -j $jobs test data/vw/$tag_type/{1/.}.vw $model_out_path/{1/.}.model ::: $input_files

