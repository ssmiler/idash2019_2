import sys

if len(sys.argv) < 2:
    sys.exit(-1)
filename = sys.argv[1]
prefix = sys.argv[2] if len(sys.argv) == 3 else ''

lines = open(filename).readlines()

fields = dict(ram = [], tot_time = [], ser_time = [])
for line in lines:
    line = line.strip()
    sline = line.split()
    if line.startswith('NUM_SAMPLES:'):
        fields['nb_samples'] = sline[-1]
    elif line.startswith('NUM_TARGET_POSITIONS:'):
        fields['nb_targets'] = sline[-1]
    elif line.startswith('NUM_TAG_POSITIONS:'):
        fields['nb_tags'] = sline[-1]
    elif line.startswith('Number of threads'):
        fields['nb_threads'] = sline[-1]
    elif line.startswith('Total wall time (seconds)'):
        fields['kg_tot'] = sline[-1]
    elif line.startswith('Keygen time (seconds)'):
        fields['kg_fhe'] = sline[-1]
    elif line.startswith('RAM usage (MB)'):
        fields['ram'].append(sline[-1])
    elif line.startswith('encrypt wall time (seconds)'):
        fields['enc_fhe'] = sline[-1]
    elif line.startswith('serialization wall time (seconds)'):
        fields['ser_time'].append(sline[-1])
    elif line.startswith('total wall time (seconds)'):
        fields['tot_time'].append(sline[-1])
    elif line.startswith('fhe wall time (seconds)'):
        fields['cld_fhe'] = sline[-1]
    elif line.startswith('decrypt wall time (seconds)'):
        fields['dec_fhe'] = sline[-1]
    elif line.endswith('encrypted_data.bin'):
        fields['enc_inp_size'] = float(sline[-5])/1024/1024
    elif line.endswith('encrypted_prediction.bin'):
        fields['enc_out_size'] = float(sline[-5])/1024/1024


assert(len(fields['ram']) == 4)
assert(len(fields['ser_time']) == 3)
assert(len(fields['tot_time']) == 3)

fields['kg_ram'], fields['enc_ram'], fields['cld_ram'], fields['dec_ram'] = fields['ram']
del fields['ram']

fields['enc_ser'], fields['cld_ser'], fields['dec_ser'] = fields['ser_time']
del fields['ser_time']

fields['enc_tot'], fields['cld_tot'], fields['dec_tot'] = fields['tot_time']
del fields['tot_time']


#print('{},{kg_tot},{kg_ram},{enc_fhe},{enc_ser},{enc_tot},{enc_ram},{cld_fhe},{cld_ser},{cld_tot},{cld_ram},{dec_fhe},{dec_ser},{dec_tot},{dec_ram}'.format(prefix, **fields))
#print('{},{kg_tot},{kg_ram},{enc_fhe},{enc_ser},{enc_tot},{enc_ram},{cld_fhe},{cld_ser},{cld_tot},{cld_ram},{dec_fhe},,,{dec_ram}'.format(prefix, **fields))
print('{},{kg_fhe},{kg_tot},{kg_ram},{enc_fhe},{enc_ser},{enc_ram},{cld_fhe},{cld_ser},{cld_ram},{dec_fhe},{dec_ser},{dec_ram},{enc_inp_size:.2f},{enc_out_size:.2f}'.format(prefix, **fields))
