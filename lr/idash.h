#ifndef IDASH_2019_IDASH_H
#define IDASH_2019_IDASH_H

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <tlwe.h>

#define REQUIRE_DRAMATICALLY(cond, message) if (!(cond)) { std::cout << "ERROR: " << message << std::endl; abort(); }

#define DIE_DRAMATICALLY(message) { std::cout << "ERROR: " << message << std::endl; abort(); }


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

void write_decrypted_predictions(const DecryptedPredictions &predictions, const std::string &filename);

void read_decrypted_predictions(DecryptedPredictions &predictions, const std::string &filename);

void encrypt_data(EncryptedData &enc_data, const PlaintextData &plaintextData, const TLweKey *key);

void cloud_compute_score(EncryptedPredictions &enc_preds, const EncryptedData &enc_data, const Model &model);

void decrypt_predictions(DecryptedPredictions &predictions, const EncryptedData &enc_preds, const TLweKey *key);

double compute_auc(const DecryptedPredictions &predictions, const PlaintextData &actual_values);

#endif //IDASH_2019_IDASH_H
