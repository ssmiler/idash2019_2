## Genome imputation using logistic regression

Genome imputation allows to predict missing variant genotypes (target SNPs) from available genotypes (tag SNPs).
In this project, the imputation is performed using logistic regression models.
The models are build upon one-hot-encoded tag SNPs and allow to impute target SNP variants.
The [vowpal-wabbit](https://github.com/VowpalWabbit/vowpal_wabbit) library is used for training models.

Input genome data files must be located in folder is `./orig_data` (copy them into this folder).
The obtained models will be located under folder `./models/hr/neighbors=<neighbors><population>` where `<neighbors>` and `<population>` are model configuration parameters.


### Prerequisites

You can either install the needed packages on your machine or use a docker container for.
Please refer to respective section.

#### Machine configuration (ubuntu 18.04)

Install required packages:
```bash
apt-get install vowpal-wabbit python3 python3-pip parallel
pip3 install numpy pandas scikit-learn
```

#### Use docker container

You can build a docker image having the necessary system configuration.
Execute the following line in the root folder of the repository:
```bash
docker build -t idash_2019_chimera:train -f Dockerfile.train .
```

Start docker container:
```bash
docker run -it --rm -v $(pwd):/idash idash_2019_chimera:train bash
```

### Input data preprocessing

The following instructions are relative to `train` folder.

Transform the format of input data and build auxiliary information files.

```bash
python3 prepare_data.py ../orig_data ../data
```

If all went well you should obtain something like this in the transformed data folder:
```bash
>> ls ../data | head
tag_test.pickle
tag_test_AMR.pickle
tag_train.pickle
...
```

### Learn a logistic regression genome imputation model

Logistic regression models for imputing target SNPs are created using script `learn_vw.sh`.
The total number of obtained models will be 3 times the number of target SNPs (3 models are built for each target SNP because of the target one-hot-encoding).

For example in order to build the models using 5 nearest tag SNP neighbors (variable `neighbors`) for all population (variable `population`) use:

```bash
neighbors=5
population=''

bash learn_vw.sh $neighbors $population
```

The number of neighboring tag SNPs to use in each model training is configurable (chose from 5 to 50 neighbors).
For the population stratification you can choose on of the following values: `'' '_AFR' '_AMR' '_EUR'`.

Accuracy metrics (micro-AUC) for the obtained models are obtained like (keep double quotation marks around `*.hr`):
```bash
python3 test_vw_hr.py -m ../models/vw/neighbors=$neighbors$population/"*.hr" --tag_file ../data/tag_test$population.pickle --target_file ../data/target_test$population.pickle
```

The output shall look like:
```
Micro-AUC score: 0.99031058 pred max-min 39.714683 (../models/vw/neighbors=5/*.hr)
```

Here, value `39.714683` is the absolute norm of the obtained predictions and is used for rescaling and mapping models to integer coefficients.
This operation is performed using the same python script as before but with additional arguments:
```bash
range=39.714683
model_scale=$(python3 -c "print(16384 / $range / 2)") # leave a 100% margin (by 2 division)

python3 test_vw_hr.py -m ../models/vw/neighbors=$neighbors$population/"*.hr" --tag_file ../data/tag_test$population.pickle --target_file ../data/target_test$population.pickle --model_scale $model_scale --out_dir ../models/hr/neighbors=$neighbors$population
```

The output shall look like (observe that the accuracy is somewhat worse):
```
Micro-AUC score: 0.99030693 pred max-min 8191.0 (../models/vw/neighbors=5/*.hr)
```

Finally, folder `../models/hr/neighbors=$neighbors$population`{.bash} contains discretized genome imputation models which can be used in the next phase (refer to evaluation [readme](../eval/README.md)):
```bash
>> ls ../models/hr/neighbors=$neighbors$population | head
17084716_0.hr
17084716_1.hr
17084716_2.hr
17084761_0.hr
...
```

