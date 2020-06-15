#########################
# learn models
#########################

neighbours="5 10 15 20 25 30 35 40 45 50"
for population in "" "_AFR" "_AMR" "_EUR"
do
    for n in $neighbours
    do
        # learn models for different neighborhoods
        time bash learn_vw.sh $n $population

        # compute predictions score for each model
        time python3 test_vw_hr.py -m ../models/vw/neighbors=$n$population"/*.hr" --tag_file ../data/tag_test$population.pickle --target_file ../data/target_test$population.pickle >> results$population.log
    done
done

#########################
# discretize models
#########################

function test_discretize()
{
  IFS=','
  set -- $1
  population=$1
  neighbors=$2
  range=$3

  scale=16384
  model_scale=$(python3 -c "print(16384 / $range / 2)") # leave a 100% margin (by 2 division)

  # echo "Discretize: " $population $neighbors $range $scale $model_scale

  model_inp_path="../models/vw/neighbors="$neighbors$population
  model_hr_out_path="../models/hr/neighbors="$neighbors$population
  rm -rf $model_hr_out_path
  mkdir -p $model_hr_out_path
  python3 test_vw_hr.py -m "$model_inp_path/*.hr" --model_scale $model_scale --out_dir $model_hr_out_path --tag_file ../data/tag_test$population.pickle --target_file ../data/target_test$population.pickle
}
export -f test_discretize


# all population
vals=`python3 -c 'print("\n".join(map(lambda ls: ",{},{}".format(ls[-1][24:-6], ls[-2]), map(lambda l: l.strip().split(), filter(lambda l: l.startswith("Micro"), open("results.log").readlines())))))'`
parallel -j 16 -k test_discretize {} ::: $vals > results_discr.log

#  populations
for population in "_AFR" "_AMR" "_EUR"
do
  vals=`python3 -c 'print("\n".join(map(lambda ls: "{},{},{}".format(ls[-1][-10:-6], ls[-1][24:-10], ls[-2]), map(lambda l: l.strip().split(), filter(lambda l: l.startswith("Micro"), open("results'$population'.log").readlines())))))'`
  parallel -j 16 -k test_discretize {} ::: $vals > results_discr$population.log
done
