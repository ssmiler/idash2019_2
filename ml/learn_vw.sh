jobs=4

tag_type=10k
# tag_type=1k

model_out_path=model/vw/$tag_type/
mkdir -p $model_out_path

input_files=data/vw/$tag_type/16367555.vw
# input_files=data/vw/$tag_type/*.vw


params="-k "
# params="-k --quiet "
params+="--loss_function=logistic "
params+="--oaa 3 "
params+="--holdout_off "
params+="--passes 100 -c "
params+="--l1 0.000001 "
# params+="-q :: " #quadratic interactions
# params+="--feature_limit 10" #quadratic interactions

# train models
parallel split -l 2250 {1} {1} ::: $input_files # split input files into train (aa suffix) and validation sets (ab suffix)
parallel -j $jobs vw -d {1}aa $params --save_per_pass -f $model_out_path/{1/.}.model ::: $input_files

# make predictions and compute AUC score

function f()
{
  data_file=$1
  snp=${data_file%*.vw}
  snp=${snp##*/}

  # make predictions with each model
  for model in $model_out_path/$snp.model.*
  do
    vw --quiet -d $data_file -i $model -t -r $model".pred"
    val=`python3 roc_vw.py -s $snp -p $model".pred" -i 0`
    # vw --quiet -d $data_file"ab" -i $model -t -r $model".pred"
    # val=`python3 roc_vw.py -s $snp -p $model".pred" -i 2250`
    echo ${model##*.} $val
  done
  rm $model_out_path/$snp.model.*.pred
}
export -f f

parallel -j $jobs echo f {1} ::: $input_files

parallel -j $jobs vw --quiet -d {1} -i $model_out_path/{1/.}.model -t -r $model_out_path/{1/.}.model.pred ::: $input_files
parallel -j $jobs python3 roc_vw.py -s {1/.} -p $model_out_path/{1/.}.model.pred ::: $input_files

exit

# model to human-readable format
parallel -j $jobs vw --quiet -d {1} -i $model_out_path/{1/.}.model --invert_hash $model_out_path/{1/.}.model.hr -t ::: $input_files



exit

parallel -j $jobs vw -d {1} $params -f $model_out_path/{1/.}.model ::: $input_files
