tag_type=10k
# tag_type=1k

model_out_path=../model/vw/$tag_type/
mkdir -p $model_out_path

input_files=../data/vw/$tag_type/*.vw


params="-k --quiet "
params+="--loss_function=logistic "
params+="--oaa 3 "
params+="--holdout_period 5 "
params+="--passes 10 -c "
# params+="-q :: " #quadratic interactions

# train models
parallel -j 1 vw -d {1} $params -f $model_out_path/{1/.}.model ::: $input_files

# make predictions and compute AUC score
parallel -j 1 vw --quiet -d {1} -i $model_out_path/{1/.}.model -t -r $model_out_path/{1/.}.model.pred ::: $input_files
parallel -j 1 python3 roc_vw.py -s {1/.} -p $model_out_path/{1/.}.model.pred  ::: $input_files

# model to human-readable format
parallel -j 1 vw --quiet -d {1} -i $model_out_path/{1/.}.model --invert_hash $model_out_path/{1/.}.model.hr -t ::: $input_files
