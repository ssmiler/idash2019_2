# suffix=training
# train_len=1200

suffix=full
#data_suffix=''
#data_suffix='_AFR'
data_suffix='_AMR'
#data_suffix='_EUR'

train_len=`cat "../data/training_sample_ids$data_suffix.list" | wc -l`

neighbours="5 10 15 20 25 30 35 40"

for n in $neighbours
do
    # learn models for different neighborhoods
    time bash learn_vw.sh $n $train_len $data_suffix
    # compute predictions score for each model
    time python3 test_vw_hr.py -m "model/vw/neighbors="$n$data_suffix"/*_orig.hr" -i $train_len  --tag_file data/tag_$suffix$data_suffix.pickle --target_file data/target_$suffix$data_suffix.pickle > result_$n$data_suffix.log
done


exit


# all populations
# python3 -c 'print("\n".join(map(lambda ls: "vals+=\"{},{} \"".format(ls[4][19:-17], ls[-1]), map(lambda l: l.strip().split(), open("result_all.log").readlines()))))'
vals=""
vals+="5,93.519295 "
vals+="10,90.310766 "
vals+="15,90.06903619999999 "
vals+="20,91.96853229999999 "
vals+="25,92.39278308000002 "
vals+="30,94.39127009999999 "
vals+="35,92.75729504200001 "
vals+="40,92.39282061 "
vals+="45,94.03803239999999 "
vals+="50,90.16025050000002 "

# AFR
# python3 -c 'print("\n".join(map(lambda ls: "vals+=\"{},{} \"".format(ls[4][19:-21], ls[-1]), map(lambda l: l.strip().split(), open("results_AFR.log").readlines()))))'
vals=""
vals+="5,65.60234525000001 "
vals+="10,78.1859599 "
vals+="15,83.28176679999999 "
vals+="20,82.101314 "
vals+="25,82.5128391 "
vals+="30,83.79238953 "
vals+="35,82.44309315000001 "
vals+="40,81.86045391999998 "
vals+="45,81.10257222999999 "

# AMR
# python3 -c 'print("\n".join(map(lambda ls: "vals+=\"{},{} \"".format(ls[4][19:-21], ls[-1]), map(lambda l: l.strip().split(), open("results_AMR.log").readlines()))))'
vals=""

# EUR
# python3 -c 'print("\n".join(map(lambda ls: "vals+=\"{},{} \"".format(ls[4][19:-21], ls[-1]), map(lambda l: l.strip().split(), open("results_EUR.log").readlines()))))'
vals=""

function test_discretize()
{
  suffix=$2
  train_len=$3 
  data_suffix=$4 

  IFS=','
  set -- $1
  neighbors=$1
  range=$2

  scale=16384
  model_scale=$(echo "$scale / 2. / $range" | bc -l)

  #echo $neighbors $range $scale $model_scale
  
  model_inp_path="model/vw/neighbors="$neighbors$data_suffix
  model_hr_out_path="model/hr/neighbors="$neighbors"_scale="$scale$data_suffix
  rm -rf $model_hr_out_path
  mkdir -p $model_hr_out_path
  python3 test_vw_hr.py -m "$model_inp_path/*_orig.hr" --model_scale $model_scale --out_dir $model_hr_out_path -i $train_len  --tag_file data/tag_$suffix$data_suffix.pickle --target_file data/target_$suffix$data_suffix.pickle
}
export -f test_discretize

parallel -j 10 -k test_discretize {} $suffix $train_len ::: $vals

