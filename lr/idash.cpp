#include <fstream>
#include <sstream>
#include "idash.h"
#include "parse_vw.h"

using namespace std;


//const uint32_t IdashParams::NUM_REGIONS = 2;
//const uint32_t IdashParams::NUM_SNP_PER_POSITIONS = 3;

//TRLWE parameters
//const uint32_t IdashParams::N = 1024;              // TRLWE dimension
//const uint32_t IdashParams::k = 1;                 // k is always 1
const double IdashParams::alpha = pow(2., -20.);   // minimal noise   //TODO check the correct value for alpha
//const uint32_t IdashParams::REGION_SIZE = IdashParams::N / IdashParams::NUM_REGIONS;           // N / NUM_REGIONS must be >= NUM_SAMPLES
const TLweParams *IdashParams::tlweParams = new_TLweParams(IdashParams::N, IdashParams::k, IdashParams::alpha, 0.25);

// SCALING_FACTOR is chosen at encryption
// enc_data = one hot encoding of input / SCALING_FACTOR
//            indexed by input feature name_snp: pos_0, pos_1, pos_2
//            1 TRLWE packs the N samples
const double IdashParams::IN_SCALING_FACTOR = 100;   // upon encryption, scale by IN_SCALING_FACTOR : double -> [-2^31,2^31[ //TODO check with Sergiu

const Torus32 IdashParams::ONE_IN_T32 = Torus32(rint(IN_SCALING_FACTOR));
const Torus32 IdashParams::NAN_0_IN_T32 = Torus32(
        rint(3. / 6. * IN_SCALING_FACTOR));  // one hot encoding of NAN - value for snp 0
const Torus32 IdashParams::NAN_1_IN_T32 = Torus32(
        rint(2. / 6. * IN_SCALING_FACTOR));  // one hot encoding of NAN - value for snp 1
const Torus32 IdashParams::NAN_2_IN_T32 = Torus32(
        rint(1. / 6. * IN_SCALING_FACTOR));  // one hot encoding of NAN - value for snp 2



/**
 * @brief      Split a string in format "pos_val" into a pair <pos,val>
 */
pair<string, uint8_t> split(const string &buf) {
    auto k = buf.find('_');

    string snp = buf.substr(0, k);
    uint8_t val = stoi(buf.substr(k + 1));
    return make_pair(snp, val);
}

/**
 * @brief      Read models given in @c params.out_features_index structure
 *
 * @param      model   The model structure
 * @param[in]  params  The parameters
 * @param[in]  path    The path
 */
void read_model(Model &model, const IdashParams &params, const string &path) {
    for (const auto &elem: params.out_features_index) {
        string pos = elem.first;
        auto out_bindices = elem.second;

        for (int snp = 0; snp < 3; ++snp) {
            string file_name = path + "/" + pos + "_" + to_string(snp) + ".hr";
            unordered_map<string, int32_t> coefs = read(file_name);

            FeatBigIndex out_bidx = out_bindices[snp];
            auto &model_coefs = model.model[out_bidx] = unordered_map<FeatBigIndex, int32_t>();

            // transform raw coefficients to BigIndex
            for (const auto &elem : coefs) {
                auto pos_snp_inp = split(elem.first);
                FeatBigIndex feat_bidx = params.inBigIdx(pos_snp_inp.first, pos_snp_inp.second);
                model_coefs[feat_bidx] = elem.second;
            }
        }
    }

    // // go through all files on path
    // for (const auto & entry : fs::directory_iterator(path)) {
    //   // parse only files with ".hr" extension
    //   if (entry.path().extension() == ".hr") {
    //     unordered_map<string, int32_t> coefs;
    //     read(coefs, entry.path());

    //     // split file name into <snp, val> pair and get its BigIndex
    //     auto pos_snp_out = split(entry.path().stem());
    //     FeatBigIndex out_idx = params.outBigIdx(pos_snp_out.first, pos_snp_out.second);

    //     auto& model_coefs = model.model[out_idx] = unordered_map<FeatBigIndex, int32_t>();

    //     // transform raw coefficients to BigIndex
    //     for (const auto& elem : coefs) {
    //       auto pos_snp_inp = split(elem.first);
    //       FeatBigIndex inp_idx = params.inBigIdx(pos_snp_inp.first, pos_snp_inp.second);

    //       model_coefs[inp_idx] = elem.second;
    //     }
    //   }
    // }
}

IdashKey::IdashKey(const IdashParams *idashParams, const TLweKey *tlweKey) : idashParams(idashParams),
                                                                             tlweKey(tlweKey) {}


void read_plaintext_data(PlaintextData &plaintext_data, const IdashParams &idashParams, const std::string &filename) {
    ifstream challenge(filename);
    std::string line;
    int64_t numPositionsRead = 0;
    for (std::getline(challenge, line); challenge; std::getline(challenge, line)) {
        std::istringstream iss(line);
        int blah = 0; // must be 1 in the file
        std::string position;
        std::string position2;
        std::string featureName;
        iss >> blah;
        REQUIRE_DRAMATICALLY(blah == 1, "file format error");
        iss >> position;
        iss >> position2;
        iss >> featureName;
        REQUIRE_DRAMATICALLY(iss, "file format error");
        std::vector<int8_t> &v = plaintext_data.data[position];
        v.resize(idashParams.NUM_SAMPLES);
        for (uint64_t sampleId = 0; sampleId < idashParams.NUM_SAMPLES; ++sampleId) {
            uint8_t snp;
            iss >> snp;
            REQUIRE_DRAMATICALLY(iss && (snp == 0 || snp == 1 || snp == 2), "file format error");
            v[sampleId] = snp;
        }
        numPositionsRead++;
    }
    REQUIRE_DRAMATICALLY(numPositionsRead == idashParams.NUM_INPUT_POSITIONS, "missing positions in plaintext file");
}


IdashKey *keygen(const std::string &targetFile, const std::string &challengeFile) {

    IdashParams *idashParams = new IdashParams();
    const TLweKey *tlweKey;

    std::string line;

    //read all output positions from the targetFile
    // initializes: NUM_OUTPUT_POSITIONS, NUM_OUTPUT_FEATURES, out_bidx_map
    std::ifstream target(targetFile);
    idashParams->NUM_OUTPUT_POSITIONS = 0;
    idashParams->NUM_OUTPUT_FEATURES = 0;
    for (std::getline(target, line); target; std::getline(target, line)) {
        std::istringstream iss(line);
        int blah = 0; // must be 1 in the file
        std::string position;
        iss >> blah;
        REQUIRE_DRAMATICALLY(blah == 1, "file format error");
        iss >> position;
        REQUIRE_DRAMATICALLY(iss, "file format error");
        idashParams->registerOutBigIdx(position, 0, idashParams->NUM_OUTPUT_FEATURES++);
        idashParams->registerOutBigIdx(position, 1, idashParams->NUM_OUTPUT_FEATURES++);
        idashParams->registerOutBigIdx(position, 2, idashParams->NUM_OUTPUT_FEATURES++);
        idashParams->NUM_OUTPUT_POSITIONS++;
    }
    target.close();

    //read all output positions from the challengeFile
    // initializes: NUM_OUTPUT_POSITIONS, NUM_OUTPUT_FEATURES, out_bidx_map
    std::ifstream challenge(challengeFile);
    idashParams->NUM_SAMPLES = 0;
    idashParams->NUM_INPUT_POSITIONS = 0;
    idashParams->NUM_INPUT_FEATURES = 0;
    for (std::getline(challenge, line); challenge; std::getline(challenge, line)) {
        std::istringstream iss(line);
        int blah = 0; // must be 1 in the file
        std::string position;
        iss >> blah;
        REQUIRE_DRAMATICALLY(blah == 1, "file format error");
        iss >> position;
        REQUIRE_DRAMATICALLY(iss, "file format error");
        idashParams->registerInBigIdx(position, 0, idashParams->NUM_INPUT_FEATURES++);
        idashParams->registerInBigIdx(position, 1, idashParams->NUM_INPUT_FEATURES++);
        idashParams->registerInBigIdx(position, 2, idashParams->NUM_INPUT_FEATURES++);
        idashParams->NUM_INPUT_POSITIONS++;
        if (idashParams->NUM_SAMPLES == 0) {
            std::string position2;
            std::string featureName;
            iss >> position2;
            iss >> featureName;
            REQUIRE_DRAMATICALLY(iss, "file format error");
            while (true) {
                uint8_t snp;
                iss >> snp;
                if (!iss) break;
                REQUIRE_DRAMATICALLY(snp == 0 || snp == 1 || snp == 2, "file format error");
                idashParams->NUM_SAMPLES++;
            }
        }
        idashParams->NUM_INPUT_FEATURES++; //for the constant value
    }
    challenge.close();


    //TRLWE parameters
    idashParams->tlweParams = new_TLweParams(idashParams->N, idashParams->k, idashParams->alpha, 0.25);


    REQUIRE_DRAMATICALLY(idashParams->REGION_SIZE >= idashParams->NUM_SAMPLES, "REGION_SIZE must be >= NUM_SAMPLES ");

    tlweKey = new_TLweKey(idashParams->tlweParams);
    IdashKey *key = new IdashKey(idashParams, tlweKey);
    return key;
}

void write_decrypted_predictions(const DecryptedPredictions &predictions, const IdashParams &params,
                                 const std::string &filename) {
    ofstream out(filename);
    out << "Subject ID,target SNP,0,1,2" << endl;
    for (uint64_t sampleId = 0; sampleId < params.NUM_SAMPLES; ++sampleId) {
        for (const auto &it : predictions.score) {
            const std::string &position = it.first;
            out << sampleId << ",";
            out << position << ",";
            out << it.second[0][sampleId] << ",";
            out << it.second[1][sampleId] << ",";
            out << it.second[2][sampleId] << endl;
        }
    }
    out.close();
}


