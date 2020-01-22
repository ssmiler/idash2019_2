scale=16384

suffix=training
train_len=1200

# suffix=full
# train_len=1500

# learn models for different neighborhoods
for n in 20 30 40 50 60 70 80 90 
do
    bash learn_vw.sh $n
done

# compute predictions score for each model
vw_paths=model/vw/neighbors\=??
parallel -j 10 -k python3 test_vw_hr.py -m {1}/*_?_orig.hr -i $train_len  --tag_file data/tag_$suffix.pickle --target_file data/target_$suffix.pickle ::: $vw_paths

exit

neighbors=25
bash learn_vw.sh $neighbors -0

model_inp_path="model/vw/neighbors="$neighbors"_"final
model_hr_out_path="model/hr/neighbors="$neighbors"_scale="$scale"_"final
rm -rf $model_hr_out_path
mkdir -p $model_hr_out_path
python3 test_vw_hr.py -m $model_inp_path/*.hr --scale $scale --out_dir $model_hr_out_path


