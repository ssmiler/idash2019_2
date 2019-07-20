#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <iostream>
#include <cmath>
#include <tfhe.h>
#include "parse_vw.h"

using namespace std;

struct PlaintextData {
    // for each feature, the snip vector (0,1,2) or -1 if NAN
    // features are indexed by name (pos)
    std::unordered_map<std::string, std::vector<int8_t>> data;
};

struct Model {
    //for each output feature and snip, coefficients map to apply
    // output features are indexed by name (pos)
    // input features are indexed by pos_0, pos_1, pos_2 or "CONSTANT"
    std::unordered_map<std::string, std::array<std::unordered_map<std::string, float>, 3>> model;
};

struct EncryptedData {
    // SCALING_FACTOR is chosen at encryption
    // enc_data = one hot encoding of input / SCALING_FACTOR
    //            indexed by input feature name_snp: pos_0, pos_1, pos_2
    //            1 TRLWE packs the N samples
    double IN_SCALING_FACTOR;   // upon encryption, scale by IN_SCALING_FACTOR
    double COEF_SCALING_FACTOR; // multiply all coeffs by COEFF_SCALING_FACTOR
    double OUT_SCALING_FACTOR;  // upon decryption, scale by OUT_SCALING_FACTOR
    // the product of the three factors is = 1.
    double NAN_0 = 3. / 6.;  // one hot encoding of NAN - value for snp 0
    double NAN_1 = 2. / 6.;  // one hot encoding of NAN - value for snp 1
    double NAN_2 = 1. / 6.;  // one hot encoding of NAN - value for snp 2
    std::unordered_map<std::string, TLweSample *> enc_data;
};

struct EncryptedPredictions {
    // predictions: for each output feature and snip, 1 TRLWE packing the N samples
    std::unordered_map<std::string, std::array<TLweSample *, 3> > score;
};

struct DecryptedPredictions {
    // predictions: for each output feature and snip, 1 vector containing the score of the N samples
    // output features are indexed by name (pos)
    std::unordered_map<std::string, std::array<std::vector<float>, 3> > score;
};

void read_model(Model &model, const std::string &filename);

void read_plaintext_data(PlaintextData &plaintext_data, const std::string &filename);

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

    PlaintextData plain_data;
    EncryptedData enc_data;
    EncryptedPredictions enc_predict;
    {
        std::unordered_map<std::string, TorusPolynomial *> raw_data;
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
            for (uint64_t i = 0; i < NumSamples; i++) {

            }
        }
    }
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
