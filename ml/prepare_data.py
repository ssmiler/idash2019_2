import pandas as pd
import numpy as np

def read_and_transform(inp_file):
  df = pd.read_csv(inp_file, sep='\t', header=None)  # read input csv file
  df = df.drop_duplicates(subset=[1,2]).T
  df.columns = df.loc[1,:].astype(int).values         # set SNP position as column index
  df = df[4:].astype(np.int8).reset_index(drop=True)  # drop first 4 rows and transform SNP values to int8
  return df

full_ids = np.array(list(map(lambda l: l.strip(), open('../data/training_sample_ids.list').readlines())))
pop_ids_AFR = np.array(list(map(lambda l: l.strip(), open('../data/AFR_training_sample_ids.list').readlines())))
pop_ids_AMR = np.array(list(map(lambda l: l.strip(), open('../data/AMR_training_sample_ids.list').readlines())))
pop_ids_EUR = np.array(list(map(lambda l: l.strip(), open('../data/EUR_training_sample_ids.list').readlines())))

df = read_and_transform('../data/tag_training.txt')
df.to_pickle('data/tag_training.pickle')

# export population stratified datasets
df.iloc[np.isin(full_ids, pop_ids_AFR)].to_pickle('data/tag_training_AFR.pickle')
df.iloc[np.isin(full_ids, pop_ids_AMR)].to_pickle('data/tag_training_AMR.pickle')
df.iloc[np.isin(full_ids, pop_ids_EUR)].to_pickle('data/tag_training_EUR.pickle')

df = pd.concat((df, read_and_transform('../data/tag_testing.txt')), axis=0)
df.to_pickle('data/tag_full.pickle')

# export population stratified datasets
df.iloc[np.isin(full_ids, pop_ids_AFR)].to_pickle('data/tag_full_AFR.pickle')
df.iloc[np.isin(full_ids, pop_ids_AMR)].to_pickle('data/tag_full_AMR.pickle')
df.iloc[np.isin(full_ids, pop_ids_EUR)].to_pickle('data/tag_full_EUR.pickle')

print(df.shape)

full_ids = np.array(list(map(lambda l: l.strip(), open('../data/testing_sample_ids.list').readlines())))
pop_ids_AFR = np.array(list(map(lambda l: l.strip(), open('../data/AFR_testing_sample_ids.list').readlines())))
pop_ids_AMR = np.array(list(map(lambda l: l.strip(), open('../data/AMR_testing_sample_ids.list').readlines())))
pop_ids_EUR = np.array(list(map(lambda l: l.strip(), open('../data/EUR_testing_sample_ids.list').readlines())))

df = read_and_transform('../data/target_training.txt')
df.to_pickle('data/target_training.pickle')

# export population stratified datasets
df.iloc[np.isin(full_ids, pop_ids_AFR)].to_pickle('data/target_training_AFR.pickle')
df.iloc[np.isin(full_ids, pop_ids_AMR)].to_pickle('data/target_training_AMR.pickle')
df.iloc[np.isin(full_ids, pop_ids_EUR)].to_pickle('data/target_training_EUR.pickle')

df = pd.concat((df, read_and_transform('../data/target_testing.txt')), axis=0)
df.to_pickle('data/target_full.pickle')

# export population stratified datasets
df.iloc[np.isin(full_ids, pop_ids_AFR)].to_pickle('data/target_full_AFR.pickle')
df.iloc[np.isin(full_ids, pop_ids_AMR)].to_pickle('data/target_full_AMR.pickle')
df.iloc[np.isin(full_ids, pop_ids_EUR)].to_pickle('data/target_full_EUR.pickle')

print(df.shape)

open("data/target_snp","w").write("\n".join(map(str, df.columns)))

