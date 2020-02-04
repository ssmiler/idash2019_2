# suffix=training
# train_len=1200

suffix=full
train_len=1500

# learn models for different neighborhoods
for n in 10 20 30 40 50 60 70 80 90 
do
    bash learn_vw.sh $n $train_len
done

exit

# compute predictions score for each model
vw_paths=model/vw/neighbors\=??
parallel -j 10 -k python3 test_vw_hr.py -m {1}/*_?_orig.hr -i $train_len  --tag_file data/tag_$suffix.pickle --target_file data/target_$suffix.pickle ::: $vw_paths

exit

vals=""
vals+="10,90.310766 "
vals+="20,91.9685323 "
vals+="30,94.3912701 "
vals+="40,92.39282061 "
vals+="50,94.8884336 "
vals+="60,94.01410409300001 "
vals+="70,93.97800534000001 "
vals+="80,93.8814865837 "
vals+="90,94.86775632 "

function test_discretize()
{
  suffix=$2
  train_len=$3  

  IFS=','
  set -- $1
  neighbors=$1
  range=$2

  scale=16384
  model_scale=$(echo "$scale / 2. / $range" | bc -l)

  #echo $neighbors $range $scale $model_scale
  
  model_inp_path="model/vw/neighbors="$neighbors"_"final
  model_hr_out_path="model/hr/neighbors="$neighbors"_scale="$scale"_"final
  rm -rf $model_hr_out_path
  mkdir -p $model_hr_out_path
  python3 test_vw_hr.py -m "$model_inp_path/*_orig.hr" --model_scale $model_scale --out_dir $model_hr_out_path -i $train_len  --tag_file data/tag_$suffix.pickle --target_file data/target_$suffix.pickle
}
export -f test_discretize

parallel -j 10 -k test_discretize {} $suffix $train_len ::: $vals

