import argparse
import glob

parser = argparse.ArgumentParser(description='Test VW human-readable model')
parser.add_argument('-m', '--models', type=str, required=True, help='.hr model pattern')
parser.add_argument('--tag_file', type=str, default='data/tag_training.pickle', help='input tag file')
parser.add_argument('--target_file', type=str, default='data/target_training.pickle', help='input target file')
parser.add_argument('-i', '--ignore_first', type=int, default=0, help='Ignore first lines')
parser.add_argument('--model_scale', type=float, help='model scale factor')
parser.add_argument('--out_dir', type=str, help='export scaled models to this path (if given)')

args = parser.parse_args()
# args = parser.parse_args('-m 25650631_0.model.hr 25650631_1.model.hr 25650631_2.model.hr'.split())

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

def export_models(df_model):
  # output models and predictions
  for model_name, coeffs in df_model.items():
    coeffs = coeffs[coeffs.abs()>1e-6]
    file_name = "{}/{}.hr".format(args.out_dir, model_name)
    open(file_name, "w").writelines(map(lambda e: '{} {}\n'.format(*e), coeffs.items()))

  #df_pred.to_csv("{}/pred.csv".format(args.out_dir))
  #df_test.to_csv("{}/test.csv".format(args.out_dir))

def make_pred(df_model, X):
  df_model.fillna(0, inplace=True)
  X = X.loc[:,df_model.index] # select features used in the models

  if args.model_scale is not None:
    # rescale model and map to integer coefficients
    # model_scale = 16383 * 0.5 / (df_pred.max().max() - df_pred.min().min())
    df_model = (df_model * args.model_scale).round()

  df_pred = X.dot(df_model)
  return df_pred, df_model

#read one hot encoded tag data or one hot encode it
tag_cache = args.tag_file + '.cache'
if os.path.exists(tag_cache):
  X = pd.read_pickle(tag_cache)
else:
  df_tag = pd.read_pickle(args.tag_file)

  # one-hot-encode SNPs and create a dataframe with "snp_val" columns
  enc = sklearn.preprocessing.OneHotEncoder(categories = [[0, 1, 2]] * df_tag.shape[1], sparse=False)
  data = enc.fit_transform(df_tag).astype(np.int8)
  columns = list(map(lambda e: "_".join(map(str,e)), it.product(df_tag.columns, [0,1,2])))
  X = pd.DataFrame(data=data, columns=columns, index=df_tag.index)

  # add dummy intercept
  X.loc[:,'Constant'] = 1
  X.to_pickle(tag_cache)

#parse models
models = glob.glob(args.models)
df_pred = None
models_coefs = dict()
for k,model in enumerate(models):
  coefs = parse_vw_hr_model(model)
  model = os.path.basename(model).split('.')[0]
  model = model[:model.rfind('_')] # cut '_orig' suffix
  models_coefs[model] = coefs
  if len(models_coefs) > 3*500 or k+1 == len(models):
      df_model = pd.DataFrame(data=models_coefs)
      df_pred_tmp, df_model = make_pred(df_model, X)
      if args.out_dir:
          export_models(df_model)
      if df_pred is None:
          df_pred = df_pred_tmp
      else:
          df_pred = pd.concat((df_pred, df_pred_tmp), axis=1)
      models_coefs = dict()
      print("{:.2f}%".format((k+1)/len(models)*100), end='\r')

target_cache = args.target_file + '.cache'
if os.path.exists(target_cache):
  y = pd.read_pickle(target_cache)
else:
  df_target = pd.read_pickle(args.target_file)

  # one-hot-encode SNPs and create a dataframe with "snp_val" columns
  enc = sklearn.preprocessing.OneHotEncoder(categories = [[0, 1, 2]] * df_target.shape[1], sparse=False)
  data = enc.fit_transform(df_target).astype(np.int8)
  columns = list(map(lambda e: "_".join(map(str,e)), it.product(df_target.columns, [0,1,2])))
  y = pd.DataFrame(data=data, columns=columns, index=df_target.index)

  print(y.shape)

  y.to_pickle(target_cache)

df_test = y.loc[:,df_pred.columns]

score1=sklearn.metrics.roc_auc_score(df_test.iloc[args.ignore_first:], df_pred.iloc[args.ignore_first:], average='micro')
score2=sklearn.metrics.roc_auc_score(df_test, df_pred, average='micro')

#score2=0.0
print("Micro-AUC score: {:.8} ({:.8} {}) pred max-min {}".format(score1, score2, args.models, df_pred.max().max() - df_pred.min().min()))

score_20=sklearn.metrics.roc_auc_score(df_test.iloc[args.ignore_first:, :20000*3], df_pred.iloc[args.ignore_first:, :20000*3], average='micro')
score_40=sklearn.metrics.roc_auc_score(df_test.iloc[args.ignore_first:, :40000*3], df_pred.iloc[args.ignore_first:, :40000*3], average='micro')

print("\t20k - {:.8}, 40k - {:.8}".format(score_20, score_40))


