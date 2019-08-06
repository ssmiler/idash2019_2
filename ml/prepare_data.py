import pandas as pd
import numpy as np

def read_and_transform(inp_file):
  df = pd.read_csv(inp_file, sep='\t', header=None).T # read input csv file and transpose
  df.columns = df.loc[1,:].astype(int).values         # set SNP position as column index
  df = df[4:].astype(np.int8).reset_index(drop=True)  # drop first 4 rows and transform SNP values to int8
  return df

# def one_hot_encode(df):
#   d=dict()
#   for name, col in df.iteritems():
#     d[name] = str(name) + '_' + col.apply(str)
#   return pd.DataFrame(d)

df1 = read_and_transform('challenge_public/sorted_target_SNP_genotypes.txt')
df1.to_pickle('data/snp_target.pickle')

df2 = read_and_transform('challenge_public/sorted_tag_SNPs_1k_genotypes.txt')
# df2 = one_hot_encode(df2)
df2.to_pickle('data/snp_tag_1k.pickle')

df3 = read_and_transform('challenge_public/sorted_tag_SNPs_10k_genotypes.txt')
# df3 = one_hot_encode(df3)
df3.to_pickle('data/snp_tag_10k.pickle')

assert(df1.shape[0] == df2.shape[0])
assert(df1.shape[0] == df3.shape[0])
