import argparse

parser = argparse.ArgumentParser(description='Compute metrics')
parser.add_argument('-s', '--snp', type=int, help='snp')
parser.add_argument('-p', '--pred_file', type=str, help='input prediction file')
parser.add_argument('-i', '--ignore_first', type=int, default=0, help='Ignore first lines')
parser.add_argument('--target_file', type=str, default='data/snp_target.pickle', help='input target file')

args = parser.parse_args()

import pandas as pd
import numpy as np
import sklearn
import sklearn.preprocessing

y = pd.read_pickle(args.target_file).loc[args.ignore_first:, args.snp]
n = y.shape[0]
y_test = np.zeros((n, 3), dtype = np.int8)
y_test[:,0] = (y == 0) + 0
y_test[:,1] = (y == 1) + 0
y_test[:,2] = (y == 2) + 0

y_pred = np.zeros((n, 3))

import sys
file = sys.stdin
if args.pred_file:
  file = open(args.pred_file)

for i in range(n):
  ls = file.readline().strip().split()
  y_pred[i] = list(map(float, ls))

import sklearn.metrics

vals = list()
for k in range(3):
  try:
    val = sklearn.metrics.roc_auc_score(y_test[:,k], y_pred[:,k])
    vals.append(val)
  except:
    vals.append("")
print(",".join(map(str, vals)), end='')

print(",{}".format(sklearn.metrics.roc_auc_score(y_test, y_pred, average='micro')), end='')
# print("{} {}".format(s_val, sklearn.metrics.log_loss(y_test, y_pred)), end='')
