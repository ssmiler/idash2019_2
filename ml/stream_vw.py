import argparse
parser = argparse.ArgumentParser(description='Stream VW for 1 SNP')
parser.add_argument('-s', '--target_snp', type=int, required=True, help='target SNP')
parser.add_argument('-n', '--neighbors', type=int, default=20, help='number of tag SNP neighbors to output')
parser.add_argument('--tag_file', type=str, default='data/snp_tag_10k.pickle', help='input tag file')
parser.add_argument('--target_file', type=str, default='data/snp_target.pickle', help='input target file')

args = parser.parse_args()
# args = parser.parse_args("-s 16367555".split())

import pandas as pd
import numpy as np

y = pd.read_pickle(args.target_file)

target_snp=args.target_snp
y=y.loc[:, target_snp]+1

X = pd.read_pickle(args.tag_file)
neighbor_tag_snps = np.abs(X.columns - target_snp).argsort()[:args.neighbors]
X = X.iloc[:, neighbor_tag_snps]

assert(len(y) == len(X))

def get_feats(x_line):
  return ' '.join(map(lambda e: '{}_{}'.format(*e), x_line.items())) # one-hot-encode features

import sys

try:
  for j in range(len(X)):
    sys.stdout.write('{} | {}\n'.format(y[j], get_feats(X.iloc[j])))
  # for j in range(len(X)):
  #   sys.stdout.write('{} | {}\n'.format(y[j], ' '.join(X.iloc[j])))
except (BrokenPipeError):
  pass

sys.stderr.close()
