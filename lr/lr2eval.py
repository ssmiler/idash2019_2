import argparse

parser = argparse.ArgumentParser(description='Get targets csv file for evaluation from LR results')
parser.add_argument('-i', '--result_file', type=str, default="result.csv", help='LR results file')
parser.add_argument('-t', '--target_file_inp', type=str, required=True, help='original target file')
parser.add_argument('-o', '--target_file_out', type=str, default="target.csv", help='output target file')

args = parser.parse_args()
# args = parser.parse_args("-t ../../data/sorted_target_SNP_genotypes_test.txt".split())

import pandas as pd
import numpy as np

def read_and_transform(inp_file):
  df = pd.read_csv(inp_file, sep='\t', header=None).T # read input csv file and transpose
  df.columns = df.loc[3,:].values                     # set SNP names as column index
  df = df[4:].astype(np.int8).reset_index(drop=True)  # drop first 4 rows and transform SNP values to int8
  return df

df_res = pd.read_csv(args.result_file)
df_res.set_index(df_res.columns[:2].tolist(), inplace=True)

df_tar = read_and_transform(args.target_file_inp).stack().loc[df_res.index]
df_tar.to_csv(args.target_file_out, index=None, header=['class'])
