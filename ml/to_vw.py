import argparse
parser = argparse.ArgumentParser(description='Output VW files')
parser.add_argument('-d', '--delta_pos', type=int, default=20, help='SNP position delta in thousands')
parser.add_argument('-o', '--out_dir', type=str, default='.', help='data output path')
parser.add_argument('-t', '--tag_file', type=str, default='data/snp_tag_1k.pickle', help='input tag file')
parser.add_argument('--target_file', type=str, default='data/snp_target.pickle', help='input target file')
parser.add_argument('-s', '--separate', action='store_true', help='export target and tag to separate files')

args = parser.parse_args()

import pandas as pd
import numpy as np

X_orig = pd.read_pickle(args.tag_file)
y_orig = pd.read_pickle(args.target_file)

def get_feats(x_line):
  return ' '.join(map(lambda e: '{}_{}'.format(*e), x_line.items())) # one-hot-encode features

def export_vw(X, y):
  assert(len(X) == len(y))
  y += 1

  lines = list()
  for j in range(len(X)):
    lines.append('{} | {}\n'.format(y[j], get_feats(X.iloc[j])))

  out_file_name = '{}/{}.vw'.format(args.out_dir, y.name)
  open(out_file_name, 'w').writelines(lines)

def export_vw_separate_tag(X):
  lines = list()
  for j in range(len(X)):
    lines.append('| {}\n'.format(get_feats(X.iloc[j])))

  import ntpath
  name = ntpath.splitext(ntpath.basename(args.tag_file))[0]
  out_file_name = '{}/{}.vw'.format(args.out_dir, name)
  open(out_file_name, 'w').writelines(lines)

def export_vw_separate_target(y):
  y += 1
  for i in range(y.shape[1]):
    target_snp = y.columns[i]
    ys = y.loc[:,target_snp]

    out_file_name = '{}/{}.target.vw'.format(args.out_dir, ys.name)
    open(out_file_name, 'w').write('\n'.join(map(str, ys.values)))


if args.separate:
  delta_pos = args.delta_pos * 1000
  for i in range(y_orig.shape[1]):
    target_snp = y_orig.columns[i]

    # select SNPs in +/-delta interval from target SNP
    ind_X = np.where((X_orig.columns > target_snp-delta_pos) & (X_orig.columns < target_snp+delta_pos))[0]
    assert(len(ind_X) == ind_X[-1] - ind_X[0] + 1)
    X_snp = X_orig.columns[ind_X]

    print('Export SNP {}'.format(target_snp))
    X, y = X_orig.loc[:,X_snp], y_orig.loc[:,target_snp]
    export_vw(X, y)
else:
  export_vw_separate_tag(X_orig)
  export_vw_separate_target(y_orig)
