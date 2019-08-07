#ifndef IDASH_2019_IDASH_H
#define IDASH_2019_IDASH_H

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <tfhe.h>

#define REQUIRE_DRAMATICALLY(cond, message) if (!(cond)) { std::cout << "ERROR: " << message << std::endl; abort(); }

#define DIE_DRAMATICALLY(message) { std::cout << "ERROR: " << message << std::endl; abort(); }

typedef uint32_t FeatBigIndex;
typedef uint32_t FeatIndex;
typedef uint32_t FeatRegion;


struct IdashParams {
    static const uint32_t NUM_REGIONS = 2;
    static const uint32_t NUM_SNP_PER_POSITIONS = 3;
    uint32_t NUM_SAMPLES;           //number of individuals
    uint32_t NUM_INPUT_POSITIONS;   //number of input features
    uint32_t NUM_OUTPUT_POSITIONS;  //number of output features
    uint32_t NUM_INPUT_FEATURES;    //number of input features (incl constant)
    uint32_t NUM_OUTPUT_FEATURES;   //number of output features

    //TRLWE parameters
    uint32_t N = 1024;              // TRLWE dimension
    uint32_t k = 1;                 // k is always 1
    double alpha = pow(2., -20.);   // minimal noise
    uint32_t REGION_SIZE;           // N / NUM_REGIONS must be >= NUM_SAMPLES

    // SCALING_FACTOR is chosen at encryption
    // enc_data = one hot encoding of input / SCALING_FACTOR
    //            indexed by input feature name_snp: pos_0, pos_1, pos_2
    //            1 TRLWE packs the N samples
    double IN_SCALING_FACTOR;   // upon encryption, scale by IN_SCALING_FACTOR : double -> [-2^31,2^31[
    double COEF_SCALING_FACTOR; // multiply all coeffs by COEFF_SCALING_FACTOR :  double -> "small int"
    double OUT_SCALING_FACTOR;  // upon decryption, scale by OUT_SCALING_FACTOR : [-2^31,2^31[ -> double
    // NOTE: the product of the three factors MUST BE = 1.
    Torus32 ONE_IN_T32 = Torus32(rint(IN_SCALING_FACTOR));
    Torus32 NAN_0_IN_T32 = Torus32(rint(3. / 6. * IN_SCALING_FACTOR));  // one hot encoding of NAN - value for snp 0
    Torus32 NAN_1_IN_T32 = Torus32(rint(2. / 6. * IN_SCALING_FACTOR));  // one hot encoding of NAN - value for snp 1
    Torus32 NAN_2_IN_T32 = Torus32(rint(1. / 6. * IN_SCALING_FACTOR));  // one hot encoding of NAN - value for snp 2


    std::unordered_map<std::string, std::array<FeatBigIndex, 3>> in_features_index;
    std::unordered_map<std::string, std::array<FeatBigIndex, 3>> out_features_index;

    inline FeatIndex feature_indexOf(uint32_t big_index) const { return big_index >> 1u; }

    inline FeatRegion feature_regionOf(uint32_t big_index) const { return big_index & 1u; }

    inline FeatBigIndex feature_bigIndexOf(uint32_t index, uint32_t region) const { return (index << 1u) | region; }

    inline FeatBigIndex constant_bigIndex() const { return -1; }

    inline FeatBigIndex inBigIdx(const std::string &pos, uint64_t snp) const {
        return in_features_index.at(pos).at(snp);
    }

    inline FeatBigIndex outBigIdx(const std::string &pos, uint64_t snp) const {
        return out_features_index.at(pos).at(snp);
    }

    inline void
    registerInBigIdx(const std::string &pos, uint64_t snp, FeatBigIndex bidx) { in_features_index[pos][snp] = bidx; }

    inline void
    registerOutBigIdx(const std::string &pos, uint64_t snp, FeatBigIndex bidx) { out_features_index[pos][snp] = bidx; }
};

struct IdashKey {
    const IdashParams *idashParams;
    const TLweParams *tlweParams;
    const TLweKey *tlweKey;
};


struct PlaintextData {
    // for each feature, the snip vector (0,1,2) or -1 if NAN
    // features are indexed by name (pos)
    std::unordered_map<std::string, std::vector<int8_t>> data;
};

struct Model {
    //for each output feature and snip, coefficients map to apply
    // output features are indexed by outBigIndex
    // input features are indexed by inBigIndex
    std::unordered_map<FeatBigIndex, std::unordered_map<FeatBigIndex, float>> model;
};

struct EncryptedData {
    std::unordered_map<FeatIndex, TLweSample *> enc_data;

    void ensure_exists(const FeatBigIndex bidx, const IdashKey &key) {
        FeatIndex featIndex = key.idashParams->feature_indexOf(bidx);
        if (enc_data.count(featIndex) == 0) {
            TLweSample *s = new_TLweSample(key.tlweParams);
            tLweSymEncryptZero(s, key.tlweParams->alpha_min, key.tlweKey);
            enc_data.emplace(featIndex, s);
        }
    }

    void setScore(FeatBigIndex bidx, uint64_t sampleId, Torus32 score, const IdashParams &params) {
        FeatIndex idx = params.feature_indexOf(bidx);
        FeatRegion rgn = params.feature_regionOf(bidx);
        enc_data.at(idx)->b->coefsT[sampleId + rgn * params.REGION_SIZE] += score;
    }
};

struct EncryptedPredictions {
    // predictions: for each output feature and snip, 1 TRLWE packing the N samples
    std::unordered_map<FeatBigIndex, TLweSample *> score;


};

struct DecryptedPredictions {
    // predictions: for each output feature and snip, 1 vector containing the score of the N samples
    // output features are indexed by name (pos)
    std::unordered_map<std::string, std::array<std::vector<float>, 3> > score;
};

void write_params(const IdashParams &params, const std::string &filename);

void read_params(IdashParams &params, const std::string &filename);

void write_key(const IdashKey &key, const std::string &filename);

void read_key(IdashKey &key, const std::string &filename);

void read_model(Model &model, const IdashParams &params, const std::string &filename);

void read_plaintext_data(PlaintextData &plaintext_data, const IdashParams &params, const std::string &filename);

void write_encrypted_data(const EncryptedData &encrypted_data, const IdashParams &params, const std::string &filename);

void read_encrypted_data(EncryptedData &encrypted_data, const IdashParams &params, const std::string &filename);

void write_encrypted_predictions(const EncryptedPredictions &encrypted_preds, const IdashParams &params,
                                 const std::string &filename);

void read_encrypted_predictions(EncryptedPredictions &encrypted_preds, const IdashParams &params,
                                const std::string &filename);

void write_decrypted_predictions(const DecryptedPredictions &predictions, const IdashParams &params,
                                 const std::string &filename);

void
read_decrypted_predictions(DecryptedPredictions &predictions, const IdashParams &params, const std::string &filename);

void encrypt_data(EncryptedData &enc_data, const PlaintextData &plain_data, const IdashKey &key);

void cloud_compute_score(EncryptedPredictions &enc_preds, const EncryptedData &enc_data, const Model &model,
                         const IdashParams &params);

void decrypt_predictions(DecryptedPredictions &predictions, const EncryptedData &enc_preds, const IdashKey &key);

double
compute_auc(const DecryptedPredictions &predictions, const PlaintextData &actual_values, const IdashParams &params);

#endif //IDASH_2019_IDASH_H