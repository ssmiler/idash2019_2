import pandas as pd
import numpy as np

df_target = pd.read_csv('../challenge_public/sorted_target_SNP_genotypes.txt', sep='\t', header=None).T
df_target.columns = df_target.loc[1,:].astype(int).values
df_target = df_target[4:].astype(np.int8).reset_index(drop=True)
df_target.to_pickle('../data/snp_target.pickle')
df_target.loc[:,df_target.columns < 1e8].to_pickle('../data/snp_target_l.pickle')
df_target.loc[:,df_target.columns > 1e8].to_pickle('../data/snp_target_h.pickle')

df_tag = pd.read_csv('../challenge_public/sorted_tag_SNPs_1k_genotypes.txt', sep='\t', header=None).T
df_tag.columns = df_tag.loc[1,:].astype(int).values
df_tag = df_tag[4:].astype(np.int8).reset_index(drop=True)
df_tag.to_pickle('../data/snp_tag_1k.pickle')
df_tag.loc[:,df_tag.columns < 1e8].to_pickle('../data/snp_tag_1k_l.pickle')
df_tag.loc[:,df_tag.columns > 1e8].to_pickle('../data/snp_tag_1k_h.pickle')

df_tag = pd.read_csv('../challenge_public/sorted_tag_SNPs_10k_genotypes.txt', sep='\t', header=None).T
df_tag.columns = df_tag.loc[1,:].astype(int).values
df_tag = df_tag[4:].astype(np.int8).reset_index(drop=True)
df_tag.to_pickle('../data/snp_tag_10k.pickle')
df_tag.loc[:,df_tag.columns < 1e8].to_pickle('../data/snp_tag_10k_l.pickle')
df_tag.loc[:,df_tag.columns > 1e8].to_pickle('../data/snp_tag_10k_h.pickle')
