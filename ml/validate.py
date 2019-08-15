import argparse

parser = argparse.ArgumentParser(description='Validate HE version')
parser.add_argument('--pred_file', type=str, required=True, help='predictions file')
parser.add_argument('--target_file', type=str, default='data/snp_target.pickle', help='target file')
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

df_target = pd.read_pickle(args.target_file)[args.ignore_first:].reset_index(drop=True)
df_target = df_target.stack()[df_pred.index]

df_target = pd.get_dummies(df_target)

print("Micro-AUC score: {}".format(sklearn.metrics.roc_auc_score(df_target, df_pred, average='micro')))
