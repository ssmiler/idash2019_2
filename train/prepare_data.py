import pandas as pd
import numpy as np

def read_and_transform(inp_file):
  df = pd.read_csv(inp_file, sep='\t', header=None)  # read input csv file
  df = df.drop_duplicates(subset=[1,2]).T
  df.columns = df.loc[1,:].astype(int).values         # set SNP position as column index
  df = df[4:].astype(np.int8).reset_index(drop=True)  # drop first 4 rows and transform SNP values to int8
  return df

def read_ids(inp_file):
    return np.array(list(map(lambda l: l.strip(), open(inp_file).readlines())))

all_ids_train = read_ids('../data/training_sample_ids.list')
pop_ids_AFR_train = read_ids('../data/AFR_training_sample_ids.list')
pop_ids_AMR_train = read_ids('../data/AMR_training_sample_ids.list')
pop_ids_EUR_train = read_ids('../data/EUR_training_sample_ids.list')

all_ids_full = np.concatenate((all_ids_train, read_ids('../data/testing_sample_ids.list')))
pop_ids_AFR_full = np.concatenate((pop_ids_AFR_train, read_ids('../data/AFR_testing_sample_ids.list')))
pop_ids_AMR_full = np.concatenate((pop_ids_AMR_train, read_ids('../data/AMR_testing_sample_ids.list')))
pop_ids_EUR_full = np.concatenate((pop_ids_EUR_train, read_ids('../data/EUR_testing_sample_ids.list')))


df = read_and_transform('../data/tag_training.txt')
df.to_pickle('data/tag_training.pickle')

# export population stratified datasets
assert(len(all_ids_train) == len(df))
df.iloc[np.isin(all_ids_train, pop_ids_AFR_train)].to_pickle('data/tag_training_AFR.pickle')
df.iloc[np.isin(all_ids_train, pop_ids_AMR_train)].to_pickle('data/tag_training_AMR.pickle')
df.iloc[np.isin(all_ids_train, pop_ids_EUR_train)].to_pickle('data/tag_training_EUR.pickle')

df = pd.concat((df, read_and_transform('../data/tag_testing.txt')), axis=0)
df.to_pickle('data/tag_full.pickle')

# export population stratified datasets
assert(len(all_ids_full) == len(df))
df.iloc[np.isin(all_ids_full, pop_ids_AFR_full)].to_pickle('data/tag_full_AFR.pickle')
df.iloc[np.isin(all_ids_full, pop_ids_AMR_full)].to_pickle('data/tag_full_AMR.pickle')
df.iloc[np.isin(all_ids_full, pop_ids_EUR_full)].to_pickle('data/tag_full_EUR.pickle')

print(df.shape)

df = read_and_transform('../data/target_training.txt')
df.to_pickle('data/target_training.pickle')

# export population stratified datasets
assert(len(all_ids_train) == len(df))
df.iloc[np.isin(all_ids_train, pop_ids_AFR_train)].to_pickle('data/target_training_AFR.pickle')
df.iloc[np.isin(all_ids_train, pop_ids_AMR_train)].to_pickle('data/target_training_AMR.pickle')
df.iloc[np.isin(all_ids_train, pop_ids_EUR_train)].to_pickle('data/target_training_EUR.pickle')

df = pd.concat((df, read_and_transform('../data/target_testing.txt')), axis=0)
df.to_pickle('data/target_full.pickle')

# export population stratified datasets
assert(len(all_ids_full) == len(df))
df.iloc[np.isin(all_ids_full, pop_ids_AFR_full)].to_pickle('data/target_full_AFR.pickle')
df.iloc[np.isin(all_ids_full, pop_ids_AMR_full)].to_pickle('data/target_full_AMR.pickle')
df.iloc[np.isin(all_ids_full, pop_ids_EUR_full)].to_pickle('data/target_full_EUR.pickle')

print(df.shape)

open("data/target_snp","w").write("\n".join(map(str, df.columns)))

