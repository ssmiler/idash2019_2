#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <iostream>
#include <cmath>
#include "parse_vw.h"

using namespace std;

struct EncryptedData {
    double SCALING_FACTOR;                              // CONSTANT chosen at encryption
    std::unordered_map<std::string, void *> enc_data;    // enc_data = one hot encoding of input / SCALING_FACTOR,
    //            1 TRLWE packs N samples
};

struct EncryptedPredictions {
    void *score[3];                                     // predictions: 3 TRLWE vector, one per snip score
    //              1 TRLWE packs N samples
};

void
read_plaintext_data(std::unordered_map<std::string, std::vector<int8_t>> &plaintext_data, const std::string &filename);

void write_encrypted_data(const EncryptedData &encrypted_data, const std::string &filename);

void read_encrypted_data(EncryptedData &encrypted_data, const std::string &filename);

void write_encrypted_predictions(const EncryptedPredictions &encrypted_preds, const std::string &filename);

void read_encrypted_predictions(EncryptedPredictions &encrypted_preds, const std::string &filename);

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


    EncryptedData enc_data;
    EncryptedPredictions enc_predict;
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

    //output score[0], score[1], score[2]
}
