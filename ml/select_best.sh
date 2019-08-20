scale=16384


tag_type=10k

# learn models for different neighborhoods
parallel -j 1 bash learn_vw.sh $tag_type {} ::: 10 20 25 30 35 40 50 60 70

# compute predictions score for each model
parallel -j 4 -k python3 test_vw_hr.py -m {1}/*_?.model.hr --tag_file data/snp_tag_$tag_type.pickle -i 2250 ::: $vw_paths

neighbors=25
bash learn_vw.sh $neighbors $tag_type -0

model_inp_path="model/vw/"$tag_type"_neighbors="$neighbors"_"final
model_hr_out_path="model/hr/"$tag_type"_neighbors="$neighbors"_scale="$scale"_"final
rm -rf $model_hr_out_path
mkdir -p $model_hr_out_path
python3 test_vw_hr.py -m $model_inp_path/*.hr --tag_file data/snp_tag_$tag_type.pickle --scale $scale --out_dir $model_hr_out_path





tag_type=1k

# learn models for different neighborhoods
parallel -j 1 bash learn_vw.sh $tag_type {} ::: 10 20 30 40 50 60 70

# compute predictions score for each model
parallel -j 4 -k python3 test_vw_hr.py -m {1}/*_?.model.hr --tag_file data/snp_tag_$tag_type.pickle -i 2250 ::: $vw_paths

neighbors=50
bash learn_vw.sh $neighbors $tag_type -0

model_inp_path="model/vw/"$tag_type"_neighbors="$neighbors"_"final
model_hr_out_path="model/hr/"$tag_type"_neighbors="$neighbors"_scale="$scale"_"final
rm -rf $model_hr_out_path
mkdir -p $model_hr_out_path
python3 test_vw_hr.py -m $model_inp_path/*.hr --tag_file data/snp_tag_$tag_type.pickle --scale $scale --out_dir $model_hr_out_path

