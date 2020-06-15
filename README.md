# Secure genome imputation using homomorphic encryption

Genome imputation allows to predict missing variant genotypes (target SNPs) from available genotypes (tag SNPs).
In this project, the imputation is performed using multi-output logistic regression models.
The models are build upon one-hot-encoded tag SNPs and are trained to predict target SNP variants (probabilities).
The [vowpal-wabbit](https://github.com/VowpalWabbit/vowpal_wabbit) framework is used for model training.

In a typical use case of secure genome imputation, we have 2 parties A and B.
Party A have genome data with missing SNPs and party B has the imputation model.
The secure genome imputation consists of 3 steps:

1. Encode/encrypt tag SNPs
2. Secure impute target SNPs
3. Decrypt target SNPs imputed probabilities

Here, steps 1 and 3 are executed by party A and step 2 is executed by party B.

The encryption, decryption and imputation use the homomorphic encryption library [TFHE](https://github.com/tfhe/tfhe).
The tag SNPs and the resulting probabilities for target SNPs are encrypted throughout the whole process.
Only party A has access to tag and target SNPs in clear.
The imputation models are available to the evaluation party B only.

The secure genome imputation is open-source software distributed under the terms of the Apache 2.0 license.

In what follows we describe how to learn imputation models and how to perform secure genome imputation.

## Learn imputation models

Input genome data files must be located in folder is `orig_data` (relative to repository root).
The obtained models will be placed under folder `models/hr/neighbors=<neighbors><population>` where `<neighbors>` and `<population>` are model configuration parameters.

### Prerequisites

You can either install the needed packages on your machine or use a docker container.
Please refer to respective section.

#### Machine configuration (ubuntu 18.04)

Install required packages:
```bash
apt-get install vowpal-wabbit python3 python3-pip parallel
pip3 install numpy pandas scikit-learn
```

#### Use docker container

You can build a docker image having the needed configuration and packages.
Execute the following command in the root folder:
```bash
docker build -t idash_2019_chimera:train -f Dockerfile.train .
```

Start docker container using:
```bash
docker run -it --rm -v $(pwd):/idash idash_2019_chimera:train bash
```

### Input data preprocessing

Next instructions are relative to `train` folder (docker container starts automatically in this folder).

Transform the format of input data and obtain auxiliary data configuration files.

```bash
python3 prepare_data.py ../orig_data ../data
```

If all went well you should obtain something like this in the output data folder:
```bash
# ls ../data | head
tag_test.pickle
tag_test_AFR.pickle
tag_test_AMR.pickle
...
```

### Logistic regression models for genome imputation

Logistic regression models for imputing target SNPs are created using script `learn_vw.sh`.
For example, in order to build the models using 5 nearest tag SNP neighbors (variable `neighbors`) for all population (variable `population`) use:

```bash
neighbors=5
population=''

bash learn_vw.sh $neighbors $population
```

Once the learning process is finished folder `../models/vw/neighbors=$neighbors$population/` will contain about 240k model files.
The total number of obtained models is 3 times the number of target SNPs (3 models are built for each target SNP because of the one-hot-encoding).
```bash
# ls ../models/vw/neighbors=$neighbors$population | wc -l
242646
```

The number of neighboring tag SNPs to use in each model training is configurable (we have tested from 5 to 50 neighbors).
For the population stratification you can choose on of the following values: `_AFR`, `_AMR`, `_EUR` or empty value for no stratification.

The micro-AUC score for the predictions obtained by these models is computed with command (keep double quotation marks around `*.hr`):
```bash
python3 test_vw_hr.py -m ../models/vw/neighbors=$neighbors$population/"*.hr" --tag_file ../data/tag_test$population.pickle --target_file ../data/target_test$population.pickle
```

The output shall look like:
```
Micro-AUC score: 0.98147822 pred max-min 89.093671 (../models/vw/neighbors=5/*.hr)
```

Here, value `89.093671` is the absolute norm of the obtained predictions and is used for model coefficient discretization (rescaling and mapping coefficients to integers).
This operation is performed using the same python script as before but with additional arguments:
```bash
range=89.093671
model_scale=$(python3 -c "print(16384 / $range / 2)") # leave a 100% margin (by 2 division)

python3 test_vw_hr.py -m ../models/vw/neighbors=$neighbors$population/"*.hr" --tag_file ../data/tag_test$population.pickle --target_file ../data/target_test$population.pickle --model_scale $model_scale --out_dir ../models/hr/neighbors=$neighbors$population
```

The output shall look like (observe that the accuracy is somewhat worse when compared to the accuracy of the non-discretized model):
```
Micro-AUC score: 0.98147786 pred max-min 8191.0 (../models/vw/neighbors=5/*.hr)
```

Discretized genome imputation models which will be used in the secure imputation phase (described in next [section](#secure-evaluation-of-imputation-models)) are placed in folder `../models/hr/neighbors=$neighbors$population`:
```bash
>> ls ../models/hr/neighbors=$neighbors$population | head
17084716_0.hr
17084716_1.hr
17084716_2.hr
17084761_0.hr
...
```

## Secure evaluation of imputation models

Once the desired logistic regression models are obtained we can proceed to the imputation on encrypted tag SNPs.

### Prerequisites

Firstly, we need to clone the TFHE library as a submodule and apply a patch to it.
Run the following instruction from the root folder of the repository:

```bash
git submodule update --init

cd tfhe
git apply ../eval/thread_local_rand_gen.patch
cd ..
```

As previously, we can either configure the host machine or use a docker container.

#### Machine configuration (ubuntu 18.04)

Python packages installed previously and usual C++ build tools (`cmake, make, g++`) are sufficient to build the project.

#### Use docker container

Execute the following command in the root folder of the repository:
```bash
docker build -t idash_2019_chimera:eval -f Dockerfile.eval .
```

Start docker container:
```bash
docker run -it --rm -v $(pwd):/idash idash_2019_chimera:eval bash
```

### Compile project

Instructions in following sections are relative to folder `eval/run` (automatically set in docker container).

Start by compiling the TFHE library and the secure genome imputation project:
```bash
make build
```

#### Number of threads

The secure imputation binaries can use parallelization in order to increase execution performance.
The default number of threads is set to 4 and can be changed (see line 37 of file [idash.h](eval/idash.h)).
The project must be re-compiled in this case.

### Execute secure imputation

Makefile target `auc` executes the key generation, encryption, missing SNPs imputation and decryption steps.
Besides, accuracy scores (micro-AUC, macro and macro non-reference accuracies) for the imputed target SNPs in output file `result_bypos.csv` are computed also.

```bash
make auc
```

The typical output on a mid-end laptop shall looks like:
```
===============================================================
./bin/keygen "../../orig_data/target_geno_model_coordinates.txt" "../../orig_data/tag_testing.txt" 1
using target file (headers): ../../orig_data/target_geno_model_coordinates.txt (only positions)
using tag file (challenge): ../../orig_data/tag_testing.txt
target_file (headers): ../../orig_data/target_geno_model_coordinates.txt
tag_file: ../../orig_data/tag_testing.txt
NUM_SAMPLES: 1004
NUM_REGIONS: 1
REGION_SIZE: 1024
NUM_TARGET_POSITIONS: 80882
NUM_TAG_POSITIONS: 16130
----------------- BENCHMARK -----------------
Number of threads ...............: 4
Keygen time (seconds)............: 5.19753e-05
Total wall time (seconds)........: 0.200243
RAM usage (MB)...................: 12.996
===============================================================
===============================================================
./bin/encrypt "../../orig_data/tag_testing.txt"
using tag file (challenge): ../../orig_data/tag_testing.txt
----------------- BENCHMARK -----------------
encrypt wall time (seconds)......: 4.62767
serialization wall time (seconds): 2.10074
total wall time (seconds)........: 6.72841
RAM usage (MB)...................: 441.484
-rw-r--r-- 1 root root 397185128 Jun 15 14:45 encrypted_data.bin
===============================================================
===============================================================
./bin/cloud "../../models/hr/neighbors=5"
using model dir: ../../models/hr/neighbors=5
----------------- BENCHMARK -----------------
fhe wall time (seconds)..........: 3.06433
serialization wall time (seconds): 26.6188
total wall time (seconds)........: 29.6832
RAM usage (MB)...................: 2735.53
-rw-r--r-- 1 root root 1991638376 Jun 15 14:46 encrypted_prediction.bin
===============================================================
===============================================================
./bin/decrypt bypos
----------------- BENCHMARK -----------------
decrypt wall time (seconds)......: 2.54242
serialization wall time (seconds): 395.26
total wall time (seconds)........: 397.802
RAM usage (MB)...................: 2957.85
===============================================================
python3 ../validate.py --pred_file result_bypos.csv --target_file "../../orig_data/target_testing.txt"
Micro-AUC score: 0.9814777758605334
MAP score: 0.8956327825366766
MAP non-ref score: 0.7289859822447496
```

The imputation model to use is set in the [Makefile-final.inc](eval/run/Makefile-final.inc) file (variable `MODEL_FILE`).
The default value corresponds to 5 neighbors and no population stratification model obtained previously.

## Paper experiments

All the experiments performed for paper [todo ref] are grouped in the bash [script](train/experiment.sh) for model learning phase and bash [script](eval/experiment.sh) for model evaluation phase.
