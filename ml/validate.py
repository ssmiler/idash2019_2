import argparse

parser = argparse.ArgumentParser(description='Validate HE version')
parser.add_argument('--pred_file', type=str, required=True, help='predictions file (.csv')
parser.add_argument('--target_file', type=str, required=True, help='target file (.csv')
parser.add_argument('-i', '--ignore_first', type=int, default=0, help='Ignore first lines')

args = parser.parse_args()

import pandas as pd
import numpy as np
import sklearn
import sklearn.metrics
import itertools as it
import os

df_pred = pd.read_csv(args.pred_file)
df_pred = df_pred.set_index(['Subject ID', 'target SNP'])
df_pred.columns = list(map(int, df_pred.columns))

nb_indiv = len(df_pred.index.levels[0])

mpreds = np.argmax(df_pred.values, axis=1).reshape(-1, nb_indiv, order='F')

def read_and_transform(inp_file):
  df = pd.read_csv(inp_file, sep='\t', header=None).T # read input csv file and transpose
  df.columns = df.loc[1,:].astype(int).values         # set SNP position as column index
  df = df[4:].astype(np.int8).reset_index(drop=True)  # drop first 4 rows and transform SNP values to int8
  return df

df_target = read_and_transform(args.target_file)
df_target = df_target[args.ignore_first:].reset_index(drop=True)
df_target = df_target.stack()[df_pred.index]

mtargets = df_target.values.reshape(-1, nb_indiv, order='F')

df_target = pd.get_dummies(df_target)

print("Micro-AUC score: {}".format(sklearn.metrics.roc_auc_score(df_target, df_pred, average='micro')))

print("MAP score: {}".format(np.mean(mpreds == mtargets)))

t = ((mtargets!=0)).sum(axis=1)
print("MAP non-ref score: {}".format(np.mean(((mtargets!=0)&(mpreds == mtargets)).sum(axis=1)[t!=0] / t[t!=0])))


