import argparse
import glob

parser = argparse.ArgumentParser(description='Test VW human-readable models')
parser.add_argument('-m', '--models', type=str,
                    required=True, help='.hr logreg model pattern')
parser.add_argument('--tag_file', type=str, help='input tag file')
parser.add_argument('--target_file', type=str, help='input target file')
parser.add_argument('--model_scale', type=float, help='model scale factor')
parser.add_argument('--out_dir', type=str,
                    help='export scaled models to this path (if given)')

args = parser.parse_args()

import pandas as pd
import numpy as np
import sklearn
import sklearn.metrics
import itertools as it
import os
import sys


def parse_vw_hr_model(file_name):
  lines = open(file_name).readlines()

  for k, line in enumerate(lines):
    if line.startswith(":0\n"):
      break

  lines = lines[k + 1:]

  model = dict()
  for line in lines:
    feat, _, coef = line.split(":")
    model[feat] = float(coef)

  return model


def export_models(df_model):
  # output models and predictions
  for model_name, coeffs in df_model.items():
    coeffs = coeffs[coeffs.abs() > 1e-6]
    file_name = "{}/{}.hr".format(args.out_dir, model_name)
    open(file_name, "w").writelines(
        map(lambda e: '{} {}\n'.format(*e), coeffs.items()))


def make_pred(df_model, X):
  df_model.fillna(0, inplace=True)
  X = X.loc[:, df_model.index]  # select features used in the models

  if args.model_scale is not None:
    # rescale model and map to integer coefficients
    df_model = (df_model * args.model_scale).round()

  df_pred = X.dot(df_model)
  return df_pred, df_model


# read one hot encoded tag data or one hot encode it
tag_cache = args.tag_file + '.cache'
if os.path.exists(tag_cache):
  X = pd.read_pickle(tag_cache)
else:
  df_tag = pd.read_pickle(args.tag_file)

  # one-hot-encode SNPs and create a dataframe with "snp_val" columns
  enc = sklearn.preprocessing.OneHotEncoder(
      categories=[[0, 1, 2]] * df_tag.shape[1], sparse=False)
  data = enc.fit_transform(df_tag).astype(np.int8)
  columns = list(map(lambda e: "_".join(map(str, e)),
                     it.product(df_tag.columns, [0, 1, 2])))
  X = pd.DataFrame(data=data, columns=columns, index=df_tag.index)

  # add dummy intercept
  X.loc[:, 'Constant'] = 1
  X.to_pickle(tag_cache)

if args.out_dir:
  os.makedirs(args.out_dir, exist_ok=True)

# parse models
models = glob.glob(args.models)
df_pred = None
models_coefs = dict()
for k, model in enumerate(models):
  coefs = parse_vw_hr_model(model)
  model = os.path.basename(model).split('.')[0]
  models_coefs[model] = coefs
  if len(models_coefs) > 3 * 500 or k + 1 == len(models):
    df_model = pd.DataFrame(data=models_coefs)
    df_pred_tmp, df_model = make_pred(df_model, X)
    if args.out_dir:
      export_models(df_model)
    if df_pred is None:
      df_pred = df_pred_tmp
    else:
      df_pred = pd.concat((df_pred, df_pred_tmp), axis=1)
    models_coefs = dict()
    print("{:.2f}%".format((k + 1) / len(models) * 100), end='\r')

if df_pred is None:
  print('Cannot find any model corresponding to given pattern "{}"'.format(args.models))
  sys.exit(-1)

target_cache = args.target_file + '.cache'
if os.path.exists(target_cache):
  y = pd.read_pickle(target_cache)
else:
  df_target = pd.read_pickle(args.target_file)

  # one-hot-encode SNPs and create a dataframe with "snp_val" columns
  enc = sklearn.preprocessing.OneHotEncoder(
      categories=[[0, 1, 2]] * df_target.shape[1], sparse=False)
  data = enc.fit_transform(df_target).astype(np.int8)
  columns = list(map(lambda e: "_".join(map(str, e)),
                     it.product(df_target.columns, [0, 1, 2])))
  y = pd.DataFrame(data=data, columns=columns, index=df_target.index)

  print('one-hot-encoded data shape', y.shape)

  y.to_pickle(target_cache)

df_test = y.loc[:, df_pred.columns]

score = sklearn.metrics.roc_auc_score(df_test, df_pred, average='micro')
print("Micro-AUC score: {:.8} pred max-min {} ({}) ".format(score,
                                                            df_pred.max().max() - df_pred.min().min(), args.models))
