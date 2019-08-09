import argparse

parser = argparse.ArgumentParser(description='Test VW human-readable model')
parser.add_argument('-m', '--models', nargs='+', type=str, required=True, help='.hr model prefix')
parser.add_argument('--tag_file', type=str, default='data/snp_tag_10k.pickle', help='input tag file')
parser.add_argument('--target_file', type=str, default='data/snp_target.pickle', help='input target file')
parser.add_argument('-i', '--ignore_first', type=int, default=0, help='Ignore first lines')
parser.add_argument('--scale', type=int, help='one-hot-encoded SNPs scaling factor, ie input features are encoded as 1/<scale>')
parser.add_argument('--out_dir', type=str, help='export scaled models to this path (if given)')

args = parser.parse_args()
# args = parser.parse_args('-m model/vw/10k/18170885_0.model.hr model/vw/10k/18170885_1.model.hr model/vw/10k/18170885_2.model.hr'.split())

args.__dict__['output_range'] = 1/2

import pandas as pd
import numpy as np
import sklearn
import sklearn.metrics
import itertools as it
import os

def parse_vw_hr_model(file_name):
  lines = open(file_name).readlines()

  for k,line in enumerate(lines):
    if line.startswith(":0\n"): break

  lines = lines[k+1:]

  model = dict()
  for line in lines:
    feat,_,coef = line.split(":")
    model[feat] = float(coef)

  return model

#parse models
models_coefs = dict()
for model in args.models:
  coefs = parse_vw_hr_model(model)
  model = os.path.basename(model).split('.')[0]
  models_coefs[model] = coefs

df_model = pd.DataFrame(data=models_coefs)
df_model.fillna(0, inplace=True)

# parse tag SNPs data
df_tag = pd.read_pickle(args.tag_file)[args.ignore_first:]
df_target = pd.read_pickle(args.target_file)[args.ignore_first:]

# one-hot-encode SNPs and create a dataframe with "snp_val" columns
enc = sklearn.preprocessing.OneHotEncoder(categories = [[0, 1, 2]] * df_tag.shape[1], sparse=False)
data = enc.fit_transform(df_tag).astype(np.int8)
columns = list(map(lambda e: "_".join(map(str,e)), it.product(df_tag.columns, [0,1,2])))
X = pd.DataFrame(data=data, columns=columns, index=df_tag.index)

# add dummy intercept
X.loc[:,'Constant'] = 1

# select only features present in the models
assert(len(df_model.index.difference(X.columns))==0)
X = X.loc[:,df_model.index]

target_snps = np.unique(df_model.columns.map(lambda e: e.split('_')[0]))

# rescale model coefficients to integers (individually per SNP model)
if args.scale:
  for target_snp in target_snps:

    # get models for a given SNP
    col_idx = df_model.columns.str.startswith(target_snp)
    assert(col_idx.sum() == 3)
    df_model_snp = df_model.loc[:, col_idx]

    # compute the 3 predictions for a SNP
    df_pred = X.dot(df_model_snp)

    # rescale model so that output range is args.out_range
    out_scale_factor = args.output_range / (df_pred.max().max() - df_pred.min().min())
    df_model_snp *= out_scale_factor

    df_model.loc[:,col_idx] = df_model_snp

  # map model to integer coefficients
  df_model = (df_model * args.scale).round().astype(int)

  # rescale features
  X /= args.scale

# make predictions
df_pred = X.dot(df_model)

# print("Model coefficients range:")
# print(df_model.describe().loc[['min','max']].T)

# print("Model output range: ")
# df = df_pred.describe().loc[['min','max']].T
# df.loc[:,'range'] = df.loc[:,'max'] - df.loc[:,'min']
# print(df)

# compute
vals_dict = dict()
for target_snp in target_snps:
  y_pred = df_pred.loc[:,df_pred.columns.str.startswith(target_snp)]
  assert(y_pred.shape[1] == 3)
  y_pred.columns = y_pred.columns.map(lambda e: int(e.split('_')[1]))

  y_test = y_pred.copy().astype(np.int8)
  y_test[:] = 0

  vals = list()
  for k in range(3):
    y_test.loc[:,k] = (df_target.loc[:,int(target_snp)] == k)+0

    try:
      val = sklearn.metrics.roc_auc_score(y_test.loc[:,k], y_pred.loc[:,k])
      vals.append(val)
    except:
      vals.append("")

  vals.append(sklearn.metrics.roc_auc_score(y_test.values, y_pred.values, average='micro'))
  vals_dict[target_snp] = dict(zip(["auc_0","auc_1","auc_2","auc"], vals))

df_vals = pd.DataFrame(vals_dict).T
print(df_vals)
print("Mean AUC score: {}".format(df_vals.auc.mean()))

if args.out_dir:
  for model_name, coeffs in df_model.items():
    coeffs = coeffs[coeffs!=0]
    file_name = "{}/{}.hr".format(args.out_dir, model_name)
    open(file_name, "w").writelines(map(lambda e: '{} {}\n'.format(*e), coeffs.items()))

