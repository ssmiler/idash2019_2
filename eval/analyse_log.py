import sys
import pandas as pd

if len(sys.argv) < 2:
    print("Usage: {} <.csv results file>".format(sys.argv[0]))

df = pd.read_csv(sys.argv[1], header=None)
df.columns = ["core_cnt", "target_cnt", "population", "neighbors_cnt", "kg_fhe", "kg_tot", "kg_ram", "enc_fhe", "enc_ser", "enc_ram", "cld_fhe", "cld_ser", "cld_ram", "dec_fhe", "dec_ser", "dec_ram", "enc_inp_size","enc_out_size"]

df.loc[df.population.isnull(), "population"] = ""

df.cld_ram /= 1024

for core_cnt, df1 in df[df.population == ''].groupby("core_cnt"):
    print()
    print("{} cores".format(df1.core_cnt.iloc[0]))
    print("#targets,KeyGen,Encryption,#neighbors,Evaluation time,Evaluation RAM (GB),Decryption,Encrypted data (MB),Predictions (MB)")
    for target_cnt, df2 in df1.groupby("target_cnt"):
        r = df2.loc[df2.neighbors_cnt == 5].iloc[0]
        print("{},{},{},{},{},{},{},{},{}".format(target_cnt, df2.kg_fhe.mean(), df2.enc_fhe.mean(), r.neighbors_cnt, r.cld_fhe, r.cld_ram, df2.dec_fhe.mean(), df2.enc_inp_size.mean(), df2.enc_out_size.mean()))

        for n in [10, 15, 20, 25, 30, 35, 40, 45, 50]:
            r = df2.loc[df2.neighbors_cnt == n].iloc[0]
            print(",,,{},{},{},,,".format(r.neighbors_cnt, r.cld_fhe, r.cld_ram))

# acceleration factor
f = lambda core_cnt, df1: [core_cnt]
for core_cnt in df.core_cnt.unique():
    print(",{}".format(*f(core_cnt,df[df.core_cnt == core_cnt])), end='')
print()

f = lambda core_cnt, df1: [df1.enc_fhe.mean()]
print("Encryption", end='')
for core_cnt in df.core_cnt.unique():
    print(",{}".format(*f(core_cnt,df[df.core_cnt == core_cnt])), end='')
print()

f = lambda core_cnt, df1: [df1.cld_fhe.mean()]
for n in [5, 10, 15, 20, 25, 30, 35, 40, 45, 50]:
    print("Eval {}".format(n), end='')
    for core_cnt in df.core_cnt.unique():
        print(",{}".format(*f(core_cnt,df[(df.core_cnt == core_cnt)&(df.neighbors_cnt == n)])), end='')
    print()


f = lambda core_cnt, df1: [df1.dec_fhe.mean()]
print("Decryption", end='')
for core_cnt in df.core_cnt.unique():
    print(",{}".format(*f(core_cnt,df[df.core_cnt == core_cnt])), end='')
print()

