# mkdir -p ../data/vw/1k && python3 -o ../data/vw/1k -t ../data/snp_tag_1k.pickle
# mkdir -p ../data/vw/1k_num && python3 -o ../data/vw/1k_num -t ../data/snp_tag_1k.pickle -n
# mkdir -p ../data/vw/10k && python3 -o ../data/vw/10k -t ../data/snp_tag_10k.pickle
# mkdir -p ../data/vw/10k_num && python3 -o ../data/vw/10k_num -t ../data/snp_tag_10k.pickle -n

import argparse
parser = argparse.ArgumentParser(description='Output VW files')
parser.add_argument('-d', '--delta_pos', type=int, default=2e5, help='SNP position delta')
parser.add_argument('-o', '--out_dir', type=str, default='.', help='data output path')
parser.add_argument('-t', '--tag_file', type=str, default='../data/snp_tag_1k.pickle', help='input tag file')
parser.add_argument('--target_file', type=str, default='../data/snp_target.pickle', help='input target file')
parser.add_argument('-n', '--numerical_feats', action='store_true', help='output numerical features, one-hot-encoded if not')

args = parser.parse_args()

import pandas as pd
import numpy as np
import sklearn
import sklearn.preprocessing


line_fmt = '{}_{}'
if args.numerical_feats:
  line_fmt = '{}:{}'

X_orig = pd.read_pickle(args.tag_file)
y_orig = pd.read_pickle(args.target_file)

def export_vw(X, y):
  assert(len(X) == len(y))

  for k in range(3):
    yp = (y == k) + 0
    if not np.any(yp): continue

    yp[yp == 0] = -1

    out_file_name = '{}/{}_{}.vw'.format(args.out_dir, y.name, k)

    lines = list()
    if args.numerical_feats:
      for j in range(len(X)):
        x_line = X.iloc[j]
        x_line = x_line[x_line != 0]
        feats = ' '.join(map(lambda e: '{}:{}'.format(*e), x_line.items())) # numerical features
        lines.append('{} | {}\n'.format(yp[j], feats))
    else:
      for j in range(len(X)):
        x_line = X.iloc[j]
        feats = ' '.join(map(lambda e: '{}_{}'.format(*e), x_line.items())) # one-hot-encoded features
        lines.append('{} | {}\n'.format(yp[j], feats))

    open(out_file_name, 'w').writelines(lines)


delta_pos = args.delta_pos
for i in range(y_orig.shape[1]):
  y_snp = y_orig.columns[i]
  ind_X = np.where((X_orig.columns >= y_snp-delta_pos) & (X_orig.columns < y_snp+delta_pos))[0]
  assert(len(ind_X) == ind_X[-1] - ind_X[0] + 1)
  X_snp = X_orig.columns[ind_X]

  X = X_orig.loc[:,X_snp]
  y = y_orig.loc[:,y_snp]

  print('Export SNP {}'.format(y_snp))
  export_vw(X, y)
