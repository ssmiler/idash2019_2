### Installation

##### Packages
`vowpal-wabbit`

##### Python 3 modules
`pandas`
`numpy`
`sklearn`


### Input data
Files `sorted_tag_SNPs_10k_genotypes.txt`, `sorted_tag_SNPs_1k_genotypes.txt` and `sorted_target_SNP_genotypes.txt` should be available in directory `../challenge_public/`

### Execution

Pickle input txt file:

`python3 prepare_data.py`

Extract neighbour tag SNPs for each target SNP:

`bash to_vw.sh`

Perform model learning with vowpal-wabbit:

`bash learn_vw.sh`

  * Change `tag_type` and `params` variables inside `learn_vw.sh` script for different
