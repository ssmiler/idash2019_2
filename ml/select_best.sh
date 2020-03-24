suffix=full
neighbours="5 10 15 20 25 30 35 40 45 50"

for data_suffix in "" "_AFR" "_AMR" "_EUR" 
do      	
    train_len=`cat "../data/training_sample_ids$data_suffix.list" | wc -l`

    for n in $neighbours
    do
        # learn models for different neighborhoods
        time bash learn_vw.sh $n $train_len $data_suffix
        # compute predictions score for each model
        time python3 test_vw_hr.py -m "model/vw/neighbors="$n$data_suffix"/*_orig.hr" -i $train_len  --tag_file data/tag_$suffix$data_suffix.pickle --target_file data/target_$suffix$data_suffix.pickle > result_$n$data_suffix.log
    done
done

exit

# all populations
# python3 -c 'print("\n".join(map(lambda ls: "vals+=\",{},{} \"".format(ls[4][19:-11], ls[-1]), map(lambda l: l.strip().split(), filter(lambda l: l.startswith("Micro"), open("results.log").readlines())))))'
vals=""
vals+=",5,93.519295 "
vals+=",10,90.310766 "
vals+=",15,90.06903619999999 "
vals+=",20,91.96853229999999 "
vals+=",25,92.39278308000002 "
vals+=",30,94.39127009999999 "
vals+=",35,92.75729504200001 "
vals+=",40,92.39282061 "
vals+=",45,94.03803239999999 "
vals+=",50,90.16025050000002 "


# python3 -c 'print("\n".join(map(lambda ls: "vals+=\"{},{},{} \"".format(ls[4][-15:-11], ls[4][19:-15], ls[-1]), map(lambda l: l.strip().split(), filter(lambda l: l.startswith("Micro"), open("results_AFR.log").readlines())))))'
# python3 -c 'print("\n".join(map(lambda ls: "vals+=\"{},{},{} \"".format(ls[4][-15:-11], ls[4][19:-15], ls[-1]), map(lambda l: l.strip().split(), filter(lambda l: l.startswith("Micro"), open("results_AMR.log").readlines())))))'
# python3 -c 'print("\n".join(map(lambda ls: "vals+=\"{},{},{} \"".format(ls[4][-15:-11], ls[4][19:-15], ls[-1]), map(lambda l: l.strip().split(), filter(lambda l: l.startswith("Micro"), open("results_EUR.log").readlines())))))'
vals=""
vals+="_AFR,5,65.60234525000001 "
vals+="_AFR,10,78.1859599 "
vals+="_AFR,15,83.28176679999999 "
vals+="_AFR,20,82.101314 "
vals+="_AFR,25,82.5128391 "
vals+="_AFR,30,83.79238953 "
vals+="_AFR,35,82.44309315000001 "
vals+="_AFR,40,81.86045391999998 "
vals+="_AFR,45,81.10257222999999 "
vals+="_AFR,50,84.10892107000001 "
vals+="_AMR,5,63.86789399999999 "
vals+="_AMR,10,66.08587849999999 "
vals+="_AMR,15,69.08228 "
vals+="_AMR,20,73.92352310000001 "
vals+="_AMR,25,70.21639605 "
vals+="_AMR,30,72.1103219 "
vals+="_AMR,35,68.750364523 "
vals+="_AMR,40,70.12886960999998 "
vals+="_AMR,45,72.63518404 "
vals+="_AMR,50,73.04694349 "
vals+="_EUR,5,66.530757 "
vals+="_EUR,10,69.49674 "
vals+="_EUR,15,75.4526682 "
vals+="_EUR,20,73.4727565 "
vals+="_EUR,25,76.89741419999999 "
vals+="_EUR,30,78.13999049999998 "
vals+="_EUR,35,77.32422276999999 "
vals+="_EUR,40,78.29400505 "
vals+="_EUR,45,79.9423129 "
vals+="_EUR,50,80.0103858 "

function test_discretize()
{
  suffix=$2

  IFS=','
  set -- $1
  data_suffix=$1
  neighbors=$2
  range=$3
    
  train_len=`cat "../data/training_sample_ids$data_suffix.list" | wc -l`
  
  scale=16384
  model_scale=$(echo "$scale / 2. / $range" | bc -l)

  echo $suffix $data_suffix $neighbors $range $train_len $scale $model_scale 

  model_inp_path="model/vw/neighbors="$neighbors$data_suffix
  model_hr_out_path="model/hr/neighbors="$neighbors"_scale="$scale$data_suffix
  rm -rf $model_hr_out_path
  mkdir -p $model_hr_out_path
  python3 test_vw_hr.py -m "$model_inp_path/*_orig.hr" --model_scale $model_scale --out_dir $model_hr_out_path -i $train_len  --tag_file data/tag_$suffix$data_suffix.pickle --target_file data/target_$suffix$data_suffix.pickle
}
export -f test_discretize

parallel -j 16 -k test_discretize {} $suffix ::: $vals

