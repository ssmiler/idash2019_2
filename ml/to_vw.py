import argparse
parser = argparse.ArgumentParser(description='Stream VW for 1 SNP')
parser.add_argument('-s', '--target_snp', type=int, required=True, help='target SNP')
parser.add_argument('-n', '--neighbors', type=int, default=20, help='number of tag SNP neighbors to output')
parser.add_argument('--tag_file', type=str, default='data/snp_tag_10k.pickle', help='input tag file')
parser.add_argument('--target_file', type=str, default='data/snp_target.pickle', help='input target file')
parser.add_argument('-o', '--out_dir', type=str, required=True, help='output path')

args = parser.parse_args()
# args = parser.parse_args("-s 16367555".split())

import pandas as pd
import numpy as np

target_snp=args.target_snp

y = pd.read_pickle(args.target_file)
y = y.loc[:, target_snp]

open('{}/{}.y'.format(args.out_dir, target_snp), "w").write("\n".join(map(str, y)))
for snp_val in range(3):
  yt=(y==snp_val)*2 - 1 # transform to -1,1 labels
  open('{}/{}_{}.y'.format(args.out_dir, target_snp, snp_val), "w").write("\n".join(map(str, yt)))

X = pd.read_pickle(args.tag_file)
neighbor_tag_snps = np.abs(X.columns - target_snp).argsort()[:args.neighbors]
X = X.iloc[:, neighbor_tag_snps]

assert(len(y) == len(X))

def get_feats(x_line):
  return ' '.join(map(lambda e: '{}_{}'.format(*e), x_line.items())) # one-hot-encode features

lines = list()
for j in range(len(X)):
  lines.append('| {}\n'.format(get_feats(X.iloc[j])))
open('{}/{}.X'.format(args.out_dir, target_snp), "w").writelines(lines)
