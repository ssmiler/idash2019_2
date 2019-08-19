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

#define CHALLENGE_FILE "../../data/sorted_tag_SNPs_1k_genotypes_test.txt"
//#define CHALLENGE_FILE "../../data/sorted_tag_SNPs_10k_genotypes_test.txt"
#define TARGET_FILE "../../data/sorted_target_SNP_genotypes.txt"


#define PARAMS_FILE "params.bin"
#define KEYS_FILE "keys.bin"
#define MODEL_FILE "../../ml/model/hr/10k"

#define ENCRYPTED_DATA_FILE "encrypted_data.bin"
#define ENCRYPTED_PREDICTION_FILE "encrypted_prediction.bin"

#define RESULT_FILE "result.csv"
#define RESULT_BYPOS_FILE "result_bypos.csv"


typedef uint32_t FeatBigIndex;
typedef uint32_t FeatIndex;
typedef uint32_t FeatRegion;


struct IdashParams {
    static const uint32_t NUM_REGIONS = 2;
    static const uint32_t NUM_SNP_PER_POSITIONS = 3;

    //TRLWE parameters
    static const uint32_t N = 1024;              // TRLWE dimension
    static const uint32_t k = 1;                 // k is always 1
    static const double alpha;   // minimal noise   //TODO check the correct value for alpha (security doc)
    static const uint32_t REGION_SIZE = N / NUM_REGIONS;           // N / NUM_REGIONS must be >= NUM_SAMPLES
    static const TLweParams *tlweParams;

    // SCALING_FACTOR is chosen at encryption
    // enc_data = one hot encoding of input / SCALING_FACTOR
    //            indexed by input feature name_snp: pos_0, pos_1, pos_2
    //            1 TRLWE packs the N samples
    static const double IN_SCALING_FACTOR;   // upon encryption, scale by IN_SCALING_FACTOR : double -> [-2^31,2^31[

    static const double ONE_IN_D;
    static const double NAN_0_IN_D;  // one hot encoding of NAN - value for snp 0
    static const double NAN_1_IN_D;  // one hot encoding of NAN - value for snp 1
    static const double NAN_2_IN_D;  // one hot encoding of NAN - value for snp 2

    static const Torus32 ONE_IN_T32;
    static const Torus32 NAN_0_IN_T32;  // one hot encoding of NAN - value for snp 0
    static const Torus32 NAN_1_IN_T32;  // one hot encoding of NAN - value for snp 1
    static const Torus32 NAN_2_IN_T32;  // one hot encoding of NAN - value for snp 2



    uint32_t NUM_SAMPLES;           //number of individuals
    uint32_t NUM_INPUT_POSITIONS;   //number of input features
    uint32_t NUM_OUTPUT_POSITIONS;  //number of output features
    uint32_t NUM_INPUT_FEATURES;    //number of input features (incl constant)
    uint32_t NUM_OUTPUT_FEATURES;   //number of output features



    std::unordered_map<uint64_t, std::array<FeatBigIndex, 3>> in_features_index;
    std::unordered_map<uint64_t, std::array<FeatBigIndex, 3>> out_features_index;
    std::vector<std::pair<uint64_t, std::string> > out_position_names; //in the order of the csv

    inline FeatIndex feature_indexOf(uint32_t big_index) const { return big_index >> 1u; }

    inline FeatRegion feature_regionOf(uint32_t big_index) const { return big_index & 1u; }

    inline FeatBigIndex feature_bigIndexOf(uint32_t index, uint32_t region) const { return (index << 1u) | region; }

    inline FeatBigIndex constant_bigIndex() const { return -1; }

    inline FeatBigIndex inBigIdx(const uint64_t &pos, uint64_t snp) const {
        return in_features_index.at(pos).at(snp);
    }

    inline FeatBigIndex outBigIdx(const uint64_t &pos, uint64_t snp) const {
        return out_features_index.at(pos).at(snp);
    }

    inline void
    registerInBigIdx(const uint64_t &pos, uint64_t snp, FeatBigIndex bidx) { in_features_index[pos][snp] = bidx; }

    inline void
    registerOutBigIdx(const uint64_t &pos, uint64_t snp, FeatBigIndex bidx) { out_features_index[pos][snp] = bidx; }

    inline void
    registerOutPositionName(const uint64_t &pos, const std::string &name) { out_position_names.push_back({pos, name}); }
};

struct IdashKey {
    const IdashParams *idashParams;
    const TLweKey *tlweKey;

    IdashKey(const IdashParams *idashParams, const TLweKey *tlweKey);
    IdashKey(){}

};


struct PlaintextData {
    // for each feature, the snip vector (0,1,2) or -1 if NAN
    // features are indexed by name (pos)
    std::unordered_map<uint64_t, std::vector<int8_t>> data;
};

struct Model {
    //for each output feature and snip, coefficients map to apply
    // output features are indexed by outBigIndex
    // input features are indexed by inBigIndex
    std::unordered_map<FeatBigIndex, std::unordered_map<FeatBigIndex, int32_t>> model;
};

struct EncryptedData {
    std::unordered_map<FeatIndex, TLweSample *> enc_data;

    void ensure_exists(const FeatBigIndex bidx, const IdashKey &key) {
        FeatIndex featIndex = key.idashParams->feature_indexOf(bidx);
        if (enc_data.count(featIndex) == 0) {
            TLweSample *s = new_TLweSample(key.idashParams->tlweParams);
            tLweSymEncryptZero(s, key.idashParams->tlweParams->alpha_min, key.tlweKey);
            enc_data.emplace(featIndex, s);
        }
    }

    void setScore(FeatBigIndex bidx, uint64_t sampleId, Torus32 score, const IdashParams &params) {
        FeatIndex idx = params.feature_indexOf(bidx);
        FeatRegion rgn = params.feature_regionOf(bidx);
        enc_data.at(idx)->b->coefsT[sampleId + rgn * params.REGION_SIZE] += score;
    }

    const TLweSample *getTLWE(FeatBigIndex inBidx, const IdashParams &params) const {
        FeatIndex idx = params.feature_indexOf(inBidx);
        REQUIRE_DRAMATICALLY(enc_data.count(idx) != 0, "shit happens before");
        return enc_data.at(idx);
    }
};

struct EncryptedPredictions {
    // predictions: for each output feature and snip, 1 TRLWE packing the N samples
    std::unordered_map<FeatBigIndex, TLweSample *> score;

    //create a (non-initialized) TLWESample at position bidx.
    TLweSample *createAndGet(FeatBigIndex bidx, const TLweParams *tLweParams) {
        const auto &it = score.find(bidx);
        if (it == score.end()) {
            TLweSample *reps = new_TLweSample(tLweParams);
            score.emplace(bidx, reps);
            return reps;
        }
        DIE_DRAMATICALLY("shit happens again"); //there should not already be a value in this map
        return it->second;
    }
};

struct DecryptedPredictions {
    // predictions: for each output feature and snip, 1 vector containing the score of the N samples
    // output features are indexed by name (pos)
    std::unordered_map<uint64_t, std::array<std::vector<float>, 3> > score;

    //verify if two DecryptedValues are equals
    static bool
    testEquals(const DecryptedPredictions &val1, const DecryptedPredictions &val2, const IdashParams &params) {

        for (uint64_t sampleId = 0; sampleId < params.NUM_SAMPLES; ++sampleId) {
            for (const auto &it : params.out_features_index) {
                const uint64_t &pos = it.first;
                if (val1.score.at(pos)[0][sampleId] != val2.score.at(pos)[0][sampleId]) {
                    std::cout << val1.score.at(pos)[0][sampleId] << " " << val2.score.at(pos)[0][sampleId] << std::endl;
                    //abort();
                };
                if (val1.score.at(pos)[1][sampleId] != val2.score.at(pos)[1][sampleId]) {
                    std::cout << val1.score.at(pos)[1][sampleId] << " " << val2.score.at(pos)[1][sampleId] << std::endl;
                    //abort();
                };
                if (val1.score.at(pos)[2][sampleId] != val2.score.at(pos)[2][sampleId]) {
                    std::cout << val1.score.at(pos)[2][sampleId] << " " << val2.score.at(pos)[2][sampleId] << std::endl;
                    //abort();
                };
            }

        }
        return true;
    }
};

void write_params(const IdashParams &params, const std::string &filename);

void read_params(IdashParams &params, const std::string &filename);

void write_key(const IdashKey &key, const std::string &filename);

void read_key(IdashKey &key, const std::string &filename);

void read_model(Model &model, const IdashParams &params, const std::string &path);

//IDASH parse idash plaintext format
void read_plaintext_data(PlaintextData &plaintext_data, const IdashParams &params, const std::string &filename);

void write_encrypted_data(const EncryptedData &encrypted_data, const IdashParams &params, const std::string &filename);

void read_encrypted_data(EncryptedData &encrypted_data, const IdashParams &params, const std::string &filename);

void write_encrypted_predictions(const EncryptedPredictions &encrypted_preds, const IdashParams &params,
                                 const std::string &filename);

void read_encrypted_predictions(EncryptedPredictions &encrypted_preds, const IdashParams &params,
                                const std::string &filename);

//IDASH csv with 3 columns of snp probabilities, comma separated
void write_decrypted_predictions(const DecryptedPredictions &predictions, const IdashParams &params,
                                 const std::string &filename, const bool PRINT_POS_NAME = true);

void encrypt_data(EncryptedData &enc_data, const PlaintextData &plain_data, const IdashKey &key);

void cloud_compute_score(EncryptedPredictions &enc_preds, const EncryptedData &enc_data, const Model &model,
                         const IdashParams &params);

void decrypt_predictions(DecryptedPredictions &predictions, const EncryptedPredictions &enc_preds, const IdashKey &key);

typedef std::map<FeatBigIndex, std::vector<double>> PlaintextOnehot;

PlaintextOnehot compute_plaintext_onehot(const PlaintextData &X, const IdashParams &params);

void
compute_score(DecryptedPredictions &predictions, const PlaintextData &X, const Model &M, const IdashParams &params);

IdashKey *keygen(const std::string &targetFile, const std::string &challengeFile);


#endif //IDASH_2019_IDASH_H
