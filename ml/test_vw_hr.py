import argparse

parser = argparse.ArgumentParser(description='Test VW human-readable model')
parser.add_argument('-m', '--models', nargs='+', type=str, required=True, help='.hr model prefix')
parser.add_argument('--tag_file', type=str, default='data/tag_training.pickle', help='input tag file')
parser.add_argument('--target_file', type=str, default='data/target_training.pickle', help='input target file')
parser.add_argument('-i', '--ignore_first', type=int, default=0, help='Ignore first lines')
parser.add_argument('--scale', type=int, help='one-hot-encoded SNPs scaling factor, ie input features are encoded as 1/<scale>')
parser.add_argument('--out_dir', type=str, help='export scaled models to this path (if given)')

args = parser.parse_args()
# args = parser.parse_args('-m 25650631_0.model.hr 25650631_1.model.hr 25650631_2.model.hr'.split())

args.__dict__['output_range'] = 1/2

import pandas as pd
import numpy as np
import sklearn
import sklearn.metrics
import itertools as it
import os

def parse_vw_hr_model(file_name):
  lines = open(file_name).readlines()

  alphas = list(map(lambda line: float(line.split()[-1]), filter(lambda line: line.startswith("alpha"), lines)))

  for k,line in enumerate(lines):
    if line.startswith(":0\n"): break

  lines = lines[k+1:]

  model = dict()
  for line in lines:
    feat,_,coef = line.split(":")
    model[feat] = float(coef)
  if len(alphas) == 0: alphas = None

  return model, alphas

#parse models
models_coefs = dict()
for model in args.models:
  coefs,alphas = parse_vw_hr_model(model)
  model = os.path.basename(model).split('.')[0]

  models_coefs[model] = dict(filter(lambda e: '[' not in e[0], coefs.items()))
  if alphas: models_coefs[model]["alpha"] = alphas[0]

  #split coefficients by individual models
  for k in range(1,20): # up to 20 models
    s = '[{}]'.format(k)
    tmp = dict(filter(lambda e: e[0].endswith(s), coefs.items()))
    if len(tmp) == 0: break
    models_coefs[model+s] = dict(map(lambda e: (e[0][:-len(s)],e[1]), tmp.items()))
    if alphas: models_coefs[model+s]["alpha"] = alphas[k]

df_model = pd.DataFrame(data=models_coefs)
df_model.fillna(0, inplace=True)

if "alpha" not in df_model.index:
  df_model.loc["alpha",:] = 1

df_model *= df_model.loc["alpha",:]
df_model.drop("alpha", axis=0, inplace=True)

# average model coefficients for same target
for target in filter(lambda e: '[' not in e, df_model.columns):
  target_cols = set(df_model.columns[df_model.columns.str.startswith(target)])
  df_model.loc[:,target] = df_model.loc[:,target_cols].mean(axis=1)
# keep only the average model
df_model = df_model.loc[:,filter(lambda e: '[' not in e, df_model.columns)]

# parse tag SNPs data
df_tag = pd.read_pickle(args.tag_file)
df_target = pd.read_pickle(args.target_file)

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

# rescale model coefficients to integers
if args.scale:
  # compute predictions
  df_pred = X.dot(df_model)

  # rescale model so that output range is args.out_range
  out_scale_factor = args.output_range / (df_pred.max().max() - df_pred.min().min())
  df_model *= out_scale_factor

  # map model to integer coefficients
  df_model = (df_model * args.scale).round().astype(int)

  # rescale features
  X /= args.scale

# make predictions
df_pred = X.dot(df_model)

# compute score
df_test = np.zeros(shape=df_pred.shape, dtype = np.int8)
df_test = pd.DataFrame(data=df_test, columns=df_pred.columns, index=df_pred.index, dtype=np.int8)
for target_snp in target_snps:
  for col in filter(lambda e: e.startswith(target_snp), df_pred.columns):
    buf = col[:-3] if '[' in col else col
    k = int(buf[len(target_snp)+1:len(target_snp)+2])
    df_test.loc[:, col] = ((df_target.loc[:,int(target_snp)] == k)+0).values.reshape(-1,1)

# print(sklearn.metrics.roc_auc_score(df_test.iloc[args.ignore_first:], df_pred.iloc[args.ignore_first:], average=None))
tmp = args.models[0].split("/")
path = "/".join(tmp[:-1])
k = tmp[-1].find("_")
k = tmp[-1].find("_", k+1)
path += "/*_?" + tmp[-1][k:]
score1=sklearn.metrics.roc_auc_score(df_test.iloc[args.ignore_first:], df_pred.iloc[args.ignore_first:], average='micro')
score2=sklearn.metrics.roc_auc_score(df_test, df_pred, average='micro')
print("Micro-AUC score: {:.8} ({:.8} {})".format(score1, score2, path))

# output models and predictions
if args.out_dir:
  for model_name, coeffs in df_model.items():
    coeffs = coeffs[coeffs!=0]
    file_name = "{}/{}.hr".format(args.out_dir, model_name)
    open(file_name, "w").writelines(map(lambda e: '{} {}\n'.format(*e), coeffs.items()))

  df_pred.to_csv("{}/pred.csv".format(args.out_dir))
  df_test.to_csv("{}/test.csv".format(args.out_dir))

