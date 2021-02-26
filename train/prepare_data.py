import pandas as pd
import numpy as np
import sys
import os

if len(sys.argv) < 3:
    print("Usage: {} <original data path> <output preprocessed data path>".format(
        sys.argv[0]))

orig_data_path = sys.argv[1]
out_data_path = sys.argv[2]

os.makedirs(out_data_path, exist_ok=True)


def read_and_transform(inp_file):
    df = pd.read_csv(inp_file, sep='\t', header=None)  # read input csv file
    df = df.drop_duplicates(subset=[1, 2]).T
    # set SNP position as column index
    df.columns = df.loc[1, :].astype(int).values
    # drop first 4 rows and transform SNP values to int8
    df = df[4:].astype(np.int8).reset_index(drop=True)
    return df


def read_ids(inp_file, assert_file_exists = True):
    if os.path.exists(inp_file):
        return np.array(list(map(lambda l: l.strip(), open(inp_file).readlines())))
    assert(not assert_file_exists), "File {} not found.".format(inp_file)
    return None


all_ids_train = read_ids(orig_data_path + '/training_sample_ids.list')
pop_ids_AFR_train = read_ids(orig_data_path + '/training_sample_ids_AFR.list', False)
pop_ids_AMR_train = read_ids(orig_data_path + '/training_sample_ids_AMR.list', False)
pop_ids_EUR_train = read_ids(orig_data_path + '/training_sample_ids_EUR.list', False)

all_ids_test = read_ids(orig_data_path + '/testing_sample_ids.list')
pop_ids_AFR_test = read_ids(orig_data_path + '/testing_sample_ids_AFR.list', False)
pop_ids_AMR_test = read_ids(orig_data_path + '/testing_sample_ids_AMR.list', False)
pop_ids_EUR_test = read_ids(orig_data_path + '/testing_sample_ids_EUR.list', False)

df = read_and_transform(orig_data_path + '/tag_training.txt')
df.to_pickle(out_data_path + '/tag_train.pickle')
print('Tag SNPs training data shape: ', df.shape)

# export population stratified datasets
assert(len(all_ids_train) == len(df))
if pop_ids_AFR_train:
    df.iloc[np.isin(all_ids_train, pop_ids_AFR_train)].to_pickle(
        out_data_path + '/tag_train_AFR.pickle')
if pop_ids_AMR_train:
    df.iloc[np.isin(all_ids_train, pop_ids_AMR_train)].to_pickle(
        out_data_path + '/tag_train_AMR.pickle')
if pop_ids_EUR_train:
    df.iloc[np.isin(all_ids_train, pop_ids_EUR_train)].to_pickle(
        out_data_path + '/tag_train_EUR.pickle')

df = read_and_transform(orig_data_path + '/tag_testing.txt')
df.to_pickle(out_data_path + '/tag_test.pickle')
print('Tag SNPs testing data shape: ', df.shape)

# export population stratified datasets
assert(len(all_ids_test) == len(df))
if pop_ids_AFR_test:
    df.iloc[np.isin(all_ids_test, pop_ids_AFR_test)].to_pickle(
        out_data_path + '/tag_test_AFR.pickle')
if pop_ids_AMR_test:
    df.iloc[np.isin(all_ids_test, pop_ids_AMR_test)].to_pickle(
        out_data_path + '/tag_test_AMR.pickle')
if pop_ids_EUR_test:
    df.iloc[np.isin(all_ids_test, pop_ids_EUR_test)].to_pickle(
        out_data_path + '/tag_test_EUR.pickle')

df = read_and_transform(orig_data_path + '/target_training.txt')
df.to_pickle(out_data_path + '/target_train.pickle')
print('Target SNPs training data shape: ', df.shape)

# export population stratified datasets
assert(len(all_ids_train) == len(df))
if pop_ids_AFR_train:
    df.iloc[np.isin(all_ids_train, pop_ids_AFR_train)].to_pickle(
        out_data_path + '/target_train_AFR.pickle')
if pop_ids_AMR_train:
    df.iloc[np.isin(all_ids_train, pop_ids_AMR_train)].to_pickle(
        out_data_path + '/target_train_AMR.pickle')
if pop_ids_EUR_train:
    df.iloc[np.isin(all_ids_train, pop_ids_EUR_train)].to_pickle(
        out_data_path + '/target_train_EUR.pickle')

df = read_and_transform(orig_data_path + '/target_testing.txt')
df.to_pickle(out_data_path + '/target_test.pickle')
print('Target SNPs testing data shape: ', df.shape)

# export population stratified datasets
assert(len(all_ids_test) == len(df))
if pop_ids_AFR_test:
    df.iloc[np.isin(all_ids_test, pop_ids_AFR_test)].to_pickle(
        out_data_path + '/target_test_AFR.pickle')
if pop_ids_AMR_test:
    df.iloc[np.isin(all_ids_test, pop_ids_AMR_test)].to_pickle(
        out_data_path + '/target_test_AMR.pickle')
if pop_ids_EUR_test:
    df.iloc[np.isin(all_ids_test, pop_ids_EUR_test)].to_pickle(
        out_data_path + '/target_test_EUR.pickle')

open(out_data_path + '/target_snp', 'w').write('\n'.join(map(str, df.columns)))
