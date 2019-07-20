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

void encryptedData_ensure_exists(EncryptedData &enc_data, const string &pos_snp, const TLweKey *key) {
    if (enc_data.enc_data.count(pos_snp) == 0) {
        TLweSample *s = new_TLweSample(key->params);
        tLweSymEncryptZero(s, key->params->alpha_min, key);
        enc_data.enc_data.emplace(pos_snp, s);
    }
}

int main() {
    std::vector<std::unordered_map<std::string, float>> model;
    read(model, "../15832228.model.hr");

    const uint64_t n = model.size();
    for (uint64_t i = 0; i < n; i++) {
        cout << i << endl;
        for (const auto &it: model[i]) {
            cout << it.first << " => " << it.second << endl;
        }
    }

    PlaintextData plain_data;
    EncryptedData enc_data;
    read_plaintext_data(plain_data, "plain_data_file");
    {
        // ============== Encrypt plaintext
        uint64_t NumSamples = 0;
        for (const auto &it: plain_data.data) {
            const std::string &pos = it.first;
            const std::vector<int8_t> &values = it.second;
            if (NumSamples == 0) {
                NumSamples = values.size();
            } else {
                REQUIRE_DRAMATICALLY(NumSamples == values.size(), "plaintext dimensions inconsistency")
            }
            encryptedData_ensure_exists(enc_data, pos + "_0", key);
            encryptedData_ensure_exists(enc_data, pos + "_1", key);
            encryptedData_ensure_exists(enc_data, pos + "_2", key);
            for (uint64_t i = 0; i < NumSamples; i++) {
                int8_t snipval = values[i];
                switch (snipval) {
                    case 0: {
                        enc_data.enc_data.at(pos + "_0")->b->coefsT[i] += Torus32(enc_data.IN_SCALING_FACTOR);
                    }
                        break;
                    case 1: {
                        enc_data.enc_data.at(pos + "_1")->b->coefsT[i] += Torus32(enc_data.IN_SCALING_FACTOR);
                    }
                        break;
                    case 2: {
                        enc_data.enc_data.at(pos + "_2")->b->coefsT[i] += Torus32(enc_data.IN_SCALING_FACTOR);
                    }
                        break;
                    default: //NAN case
                    {
                        enc_data.enc_data.at(pos + "_0")->b->coefsT[i] += Torus32(
                                enc_data.NAN_0 * enc_data.IN_SCALING_FACTOR);
                        enc_data.enc_data.at(pos + "_1")->b->coefsT[i] += Torus32(
                                enc_data.NAN_1 * enc_data.IN_SCALING_FACTOR);
                        enc_data.enc_data.at(pos + "_2")->b->coefsT[i] += Torus32(
                                enc_data.NAN_2 * enc_data.IN_SCALING_FACTOR);
                    }
                        break;
                }
            }
        }
    }
    write_encrypted_data(enc_data, "enc_data_file");


    EncryptedPredictions enc_predict;
    {
        // ============== apply model over ciphertexts
        const double &SCALING_FACTOR = enc_data.SCALING_FACTOR;
        for (uint64_t i = 0; i < n; i++) {
            // enc_predict.score[i] = TRLWE(0)
            for (const auto &it: model[i]) {
                const string &feature_name = it.first;
                const double coeff = it.second;
                const int64_t rescaled_coeff = int64_t(rint(SCALING_FACTOR * coeff));
                const void *feature_data = enc_data.enc_data.at(feature_name);
                //enc_predict.score[i] += rescaled_coeff * feature_data;
            }
        }
    }
    //output score[0], score[1], score[2]
    {
        // ============== Decrypt predictions
    }
}
