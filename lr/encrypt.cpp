#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <iostream>
#include <cmath>
#include <tfhe.h>
#include "idash.h"
#include "parse_vw.h"

using namespace std;

void encrypt_data(EncryptedData &enc_data, const PlaintextData &plain_data, const IdashKey &key) {
    const IdashParams &params = *key.idashParams;
    const uint64_t NUM_SAMPLES = params.NUM_SAMPLES;
    for (const auto &it: plain_data.data) {
        const std::string &pos = it.first;
        const std::vector<int8_t> &values = it.second;
        REQUIRE_DRAMATICALLY(values.size() == NUM_SAMPLES, "plaintext dimensions inconsistency")
        enc_data.ensure_exists(params.inBigIdx(pos, 0), key);
        enc_data.ensure_exists(params.inBigIdx(pos, 1), key);
        enc_data.ensure_exists(params.inBigIdx(pos, 2), key);
        for (uint64_t sampleId = 0; sampleId < NUM_SAMPLES; sampleId++) {
            switch (values[sampleId]) {
                case 0: {
                    enc_data.setScore(params.inBigIdx(pos, 0), sampleId, params.ONE_IN_T32, params);
                }
                    break;
                case 1: {
                    enc_data.setScore(params.inBigIdx(pos, 1), sampleId, params.ONE_IN_T32, params);
                }
                    break;
                case 2: {
                    enc_data.setScore(params.inBigIdx(pos, 2), sampleId, params.ONE_IN_T32, params);
                }
                    break;
                default: //NAN case
                {
                    enc_data.setScore(params.inBigIdx(pos, 0), sampleId, params.NAN_0_IN_T32, params);
                    enc_data.setScore(params.inBigIdx(pos, 1), sampleId, params.NAN_1_IN_T32, params);
                    enc_data.setScore(params.inBigIdx(pos, 2), sampleId, params.NAN_2_IN_T32, params);
                }
            }
        }
    }
}

int main() {
    IdashKey key;
    PlaintextData plain_data;
    EncryptedData enc_data;
    read_key(key, "key_file");
    read_plaintext_data(plain_data, *key.idashParams, "plain_data_file");
    encrypt_data(enc_data, plain_data, key);
    write_encrypted_data(enc_data, *key.idashParams, "enc_data_file");
}
