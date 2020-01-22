import pandas as pd
import numpy as np

def read_and_transform(inp_file):
  df = pd.read_csv(inp_file, sep='\t', header=None).T # read input csv file and transpose
  df.columns = df.loc[1,:].astype(int).values         # set SNP position as column index
  df = df[4:].astype(np.int8).reset_index(drop=True)  # drop first 4 rows and transform SNP values to int8
  return df

df = read_and_transform('../data/tag_training.txt')
df.to_pickle('data/tag_training.pickle')

df = pd.concat((df, read_and_transform('../data/tag_testing.txt')), axis=0)
df.to_pickle('data/tag_full.pickle')

print(df.shape)

df = read_and_transform('../data/target_training.txt')
df.to_pickle('data/target_training.pickle')

df = pd.concat((df, read_and_transform('../data/target_testing.txt')), axis=0)
df.to_pickle('data/target_full.pickle')

print(df.shape)

open("data/target_snp","w").write("\n".join(map(str, df.columns)))

