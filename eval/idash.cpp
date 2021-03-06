#include <fstream>
#include <sstream>
#include "idash.h"
#include "parse_vw.h"

#include <iostream>
#include <fstream>

#include <omp.h>

using namespace std;


//const uint32_t IdashParams::NUM_REGIONS = 2;
//const uint32_t IdashParams::NUM_SNP_PER_POSITIONS = 3;

//TRLWE parameters
//const uint32_t IdashParams::N = 1024;              // TRLWE dimension
//const uint32_t IdashParams::k = 1;                 // k is always 1
const double IdashParams::alpha = pow(2.,
                                      -25.);   // minimal noise  (around 190 bits of security regarding the standardization document)
//const uint32_t IdashParams::REGION_SIZE = IdashParams::N / IdashParams::NUM_REGIONS;           // N / NUM_REGIONS must be >= NUM_SAMPLES
const TLweParams *IdashParams::tlweParams = new_TLweParams(IdashParams::N, IdashParams::k, IdashParams::alpha, 0.25);

// SCALING_FACTOR is chosen at encryption
// enc_data = one hot encoding of input / SCALING_FACTOR
//            indexed by input feature name_snp: pos_0, pos_1, pos_2
//            1 TRLWE packs the N samples
const double IdashParams::IN_SCALING_FACTOR = 1. / 16384;   // upon encryption, scale by IN_SCALING_FACTOR : double -> [-2^31,2^31[ //TODO check with Sergiu

const double IdashParams::ONE_IN_D = IN_SCALING_FACTOR;
const double IdashParams::NAN_0_IN_D =
    3. / 6. * IN_SCALING_FACTOR; // one hot encoding of NAN - value for snp 0
const double IdashParams::NAN_1_IN_D =
    2. / 6. * IN_SCALING_FACTOR; // one hot encoding of NAN - value for snp 1
const double IdashParams::NAN_2_IN_D =
    1. / 6. * IN_SCALING_FACTOR; // one hot encoding of NAN - value for snp 2

const Torus32 IdashParams::ONE_IN_T32 = dtot32(IdashParams::ONE_IN_D);
const Torus32 IdashParams::NAN_0_IN_T32 = dtot32(
    IdashParams::NAN_0_IN_D); // one hot encoding of NAN - value for snp 0
const Torus32 IdashParams::NAN_1_IN_T32 = dtot32(
    IdashParams::NAN_1_IN_D); // one hot encoding of NAN - value for snp 1
const Torus32 IdashParams::NAN_2_IN_T32 = dtot32(
    IdashParams::NAN_2_IN_D); // one hot encoding of NAN - value for snp 2

/**
 * @brief      Split a string in format "pos_val" into a pair <pos,val>
 */
pair<uint64_t, uint8_t> split(const string &buf) {
    auto k = buf.find('_');

    string snp = buf.substr(0, k);
    uint64_t pos = stol(snp);
    uint8_t val = stoi(buf.substr(k + 1));
    return make_pair(pos, val);
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
        uint64_t pos = elem.first;
        auto out_bindices = elem.second;

        for (int snp = 0; snp < 3; ++snp) {
            string file_name = path + "/" + to_string(pos) + "_" + to_string(snp) + ".hr";
            unordered_map<string, int32_t> coefs = read(file_name);

            FeatBigIndex out_bidx = out_bindices[snp];
            auto &model_coefs = model.model[out_bidx] = unordered_map<FeatBigIndex, int32_t>();

            // transform raw coefficients to BigIndex
            for (const auto &elem : coefs) {
                if (elem.first == "Constant") {
                    FeatBigIndex feat_bidx = params.constant_bigIndex();
                    model_coefs[feat_bidx] = elem.second;
                } else {
                    auto pos_snp_inp = split(elem.first);
                    FeatBigIndex feat_bidx = params.inBigIdx(pos_snp_inp.first, pos_snp_inp.second);
                    model_coefs[feat_bidx] = elem.second;
                }
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

void ostream_write_binary(ostream &out, const void *data, const uint64_t size) {
    out.write((const char *) data, size);
}

void istream_read_binary(istream &in, void *data, const uint64_t size) {
    in.read((char *) data, size);
}


void write_params_ostream(const IdashParams &params, ostream &out) {

    ostream_write_binary(out, &params.NUM_SAMPLES, sizeof(params.NUM_SAMPLES));
    ostream_write_binary(out, &params.NUM_INPUT_POSITIONS,
                         sizeof(params.NUM_INPUT_POSITIONS));
    ostream_write_binary(out, &params.NUM_OUTPUT_POSITIONS,
                         sizeof(params.NUM_OUTPUT_POSITIONS));
    ostream_write_binary(out, &params.NUM_INPUT_FEATURES,
                         sizeof(params.NUM_INPUT_FEATURES));
    ostream_write_binary(out, &params.NUM_OUTPUT_FEATURES,
                         sizeof(params.NUM_OUTPUT_FEATURES));
    ostream_write_binary(out, &params.NUM_REGIONS,
                         sizeof(params.NUM_REGIONS));
    ostream_write_binary(out, &params.REGION_SIZE,
                         sizeof(params.REGION_SIZE));

    REQUIRE_DRAMATICALLY(params.NUM_INPUT_POSITIONS ==
                         params.in_features_index.size(),
                         "NUM_INPUT_POSITIONS != in_features_index.size()");

    for (const auto &e1 : params.in_features_index) {
        uint64_t pos = e1.first;

        // write feature position
        ostream_write_binary(out, &pos, sizeof(uint64_t));
        //size_t pos_size = pos.size();
        //ostream_write_binary(out, &pos_size, sizeof(size_t));
        //ostream_write_binary(out, pos.c_str(), sizeof(char) * pos.size());

        // write snps
        for (const FeatBigIndex e2 : e1.second) {
            ostream_write_binary(out, (&e2), sizeof(e2));
        }
    }

    REQUIRE_DRAMATICALLY(params.NUM_OUTPUT_POSITIONS ==
                         params.out_features_index.size(),
                         "NUM_OUTPUT_POSITIONS != out_features_index.size()");
    REQUIRE_DRAMATICALLY(params.NUM_OUTPUT_POSITIONS ==
                         params.out_position_names.size(),
                         "NUM_OUTPUT_POSITIONS != out_position_names.size()");

    for (const auto &e1 : params.out_position_names) {
        uint64_t pos = e1.first;
        string name = e1.second;

        // write feature position
        ostream_write_binary(out, &pos, sizeof(uint64_t));
        // write feature name (null terminated)
        ostream_write_binary(out, name.c_str(), name.size() + 1);

        // write snps
        for (const FeatBigIndex e2 : params.out_features_index.at(pos)) {
            ostream_write_binary(out, (&e2), sizeof(e2));
        }
    }


}


void write_params(const IdashParams &params, const string &filename) {
    ofstream out(filename.c_str(), ios::binary);

    REQUIRE_DRAMATICALLY(out.is_open(), "Cannot open parameters file for write");

    write_params_ostream(params, out);

    out.close();
}


void read_params_istream(IdashParams &params, istream &inp) {

    istream_read_binary(inp, &params.NUM_SAMPLES, sizeof(params.NUM_SAMPLES));
    istream_read_binary(inp, &params.NUM_INPUT_POSITIONS,
                        sizeof(params.NUM_INPUT_POSITIONS));
    istream_read_binary(inp, &params.NUM_OUTPUT_POSITIONS,
                        sizeof(params.NUM_OUTPUT_POSITIONS));
    istream_read_binary(inp, &params.NUM_INPUT_FEATURES,
                        sizeof(params.NUM_INPUT_FEATURES));
    istream_read_binary(inp, &params.NUM_OUTPUT_FEATURES,
                        sizeof(params.NUM_OUTPUT_FEATURES));
    istream_read_binary(inp, &params.NUM_REGIONS,
                         sizeof(params.NUM_REGIONS));
    istream_read_binary(inp, &params.REGION_SIZE,
                         sizeof(params.REGION_SIZE));

    for (uint32_t i = 0; i < params.NUM_INPUT_POSITIONS; ++i) {
        uint64_t pos;

        // read feature position
        istream_read_binary(inp, &pos, sizeof(uint64_t));

        // read snps
        params.in_features_index.emplace(pos, array<FeatBigIndex, 3>());
        auto &tmp = params.in_features_index.at(pos);
        for (int j = 0; j < 3; ++j) {
            istream_read_binary(inp, &tmp[j], sizeof(FeatBigIndex));
        }
    }

    REQUIRE_DRAMATICALLY(params.NUM_INPUT_POSITIONS ==
                         params.in_features_index.size(),
                         "NUM_INPUT_POSITIONS != in_features_index.size()");

    for (uint32_t i = 0; i < params.NUM_OUTPUT_POSITIONS; ++i) {
        uint64_t pos;
        string pos_name;

        // read feature position
        istream_read_binary(inp, &pos, sizeof(uint64_t));
        // read feature name (until '\0')
        for (int c = inp.get(); c > 0; c = inp.get()) {
            pos_name.push_back(c);
        }

        // read snps
        params.out_position_names.push_back({pos, pos_name});
        params.out_features_index.emplace(pos, array<FeatBigIndex, 3>());
        auto &tmp = params.out_features_index.at(pos);
        for (int j = 0; j < 3; ++j) {
            istream_read_binary(inp, &tmp[j], sizeof(FeatBigIndex));
        }
    }

    REQUIRE_DRAMATICALLY(params.NUM_OUTPUT_POSITIONS ==
                         params.out_features_index.size(),
                         "NUM_OUTPUT_POSITIONS != out_features_index.size()");
    REQUIRE_DRAMATICALLY(params.NUM_OUTPUT_POSITIONS ==
                         params.out_position_names.size(),
                         "NUM_OUTPUT_POSITIONS != out_position_names.size()");

}


void read_params(IdashParams &params, const string &filename) {
    ifstream inp(filename.c_str(), ios::binary);

    REQUIRE_DRAMATICALLY(inp.is_open(), "Cannot open parameters file for read");

    read_params_istream(params, inp);

    inp.close();
}







void read_plaintext_data(PlaintextData &plaintext_data, const IdashParams &idashParams, const std::string &filename) {
    ifstream challenge(filename);
    REQUIRE_DRAMATICALLY(challenge, "file not found");
    std::string line;
    int64_t numPositionsRead = 0;
    for (std::getline(challenge, line); challenge; std::getline(challenge, line)) {
        std::istringstream iss(line);
        int blah = 0; // must be 22 in the file
        uint64_t position;
        uint64_t position2;
        std::string featureName;
        iss >> blah;
        REQUIRE_DRAMATICALLY(blah == 22, "file format error");
        iss >> position;
        iss >> position2;
        iss >> featureName;
        REQUIRE_DRAMATICALLY(iss, "file format error");
        std::vector<int8_t> &v = plaintext_data.data[position];
        v.resize(idashParams.NUM_SAMPLES);
        string snp;
        for (uint64_t sampleId = 0; sampleId < idashParams.NUM_SAMPLES; ++sampleId) {
            iss >> snp;
            if (snp == "0" || snp == "1" || snp == "2") {
                v[sampleId] = stoi(snp);
            } else {
                v[sampleId] = -1;
            }
        }
        numPositionsRead++;
        if (numPositionsRead >= idashParams.NUM_INPUT_POSITIONS) break;
    }
    REQUIRE_DRAMATICALLY(numPositionsRead == idashParams.NUM_INPUT_POSITIONS, "missing positions in plaintext file");
}


IdashParams *keygen_ph1(const std::string &targetFile, const std::string &challengeFile, const bool targetFilePosOnly) {
    IdashParams *idashParams = new IdashParams();

    std::string line;

    //read all output positions from the targetFile
    // initializes: NUM_OUTPUT_POSITIONS, NUM_OUTPUT_FEATURES, out_bidx_map
    std::ifstream target(targetFile);
    idashParams->NUM_OUTPUT_POSITIONS = 0;
    idashParams->NUM_OUTPUT_FEATURES = 0;

    uint64_t tagPosLast; // last read tag position
    if (targetFilePosOnly) {
        uint64_t position;
        for (std::getline(target, line); target; std::getline(target, line)) {
            std::istringstream iss(line);
            std::string position_name; //must be position+1

            iss >> position;
            position_name = to_string(position);
            REQUIRE_DRAMATICALLY(iss, "file format error");

            idashParams->registerOutBigIdx(position, 0, idashParams->NUM_OUTPUT_FEATURES++);
            idashParams->registerOutBigIdx(position, 1, idashParams->NUM_OUTPUT_FEATURES++);
            idashParams->registerOutBigIdx(position, 2, idashParams->NUM_OUTPUT_FEATURES++);
            idashParams->NUM_OUTPUT_POSITIONS++;
            idashParams->registerOutPositionName(position, position_name);
        }
        tagPosLast = position;
    } else {
        uint64_t position;
        for (std::getline(target, line); target; std::getline(target, line)) {
            std::istringstream iss(line);
            int blah = 0; // must be 22 in the file
            uint64_t position2; //must be position+1
            std::string position_name; //must be position+1
            iss >> blah;
            REQUIRE_DRAMATICALLY(blah == 22, "file format error");
            iss >> position >> position2 >> position_name;
            REQUIRE_DRAMATICALLY(position2 == position + 1, "file format error");
            REQUIRE_DRAMATICALLY(iss, "file format error");
            idashParams->registerOutBigIdx(position, 0, idashParams->NUM_OUTPUT_FEATURES++);
            idashParams->registerOutBigIdx(position, 1, idashParams->NUM_OUTPUT_FEATURES++);
            idashParams->registerOutBigIdx(position, 2, idashParams->NUM_OUTPUT_FEATURES++);
            idashParams->NUM_OUTPUT_POSITIONS++;
            idashParams->registerOutPositionName(position, position_name);
        }
        tagPosLast = position;
    }

    target.close();

    //read all output positions from the challengeFile
    // initializes: NUM_OUTPUT_POSITIONS, NUM_OUTPUT_FEATURES, out_bidx_map
    std::ifstream challenge(challengeFile);
    idashParams->NUM_SAMPLES = 0;
    idashParams->NUM_INPUT_POSITIONS = 0;
    idashParams->NUM_INPUT_FEATURES = 0;
    uint64_t nbTargetsAfterLastTag = 0;
    for (std::getline(challenge, line); challenge; std::getline(challenge, line)) {
        std::istringstream iss(line);
        int blah = 0; // must be 22 in the file
        uint64_t position;
        iss >> blah;
        REQUIRE_DRAMATICALLY(blah == 22, "file format error");
        iss >> position;
        REQUIRE_DRAMATICALLY(iss, "file format error");

        if (position > tagPosLast) {
            nbTargetsAfterLastTag++;
            if (nbTargetsAfterLastTag > 50) break;
        }

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
            std::string snp;
            while (true) {
                iss >> snp;
                if (!iss) break;
                idashParams->NUM_SAMPLES++;
            }
        }
    }
    idashParams->NUM_INPUT_FEATURES++; //for the constant value
    challenge.close();

    idashParams->NUM_REGIONS = idashParams->N / idashParams->NUM_SAMPLES;
    idashParams->REGION_SIZE = idashParams->N / idashParams->NUM_REGIONS;

    REQUIRE_DRAMATICALLY(idashParams->REGION_SIZE >= idashParams->NUM_SAMPLES, "REGION_SIZE must be >= NUM_SAMPLES ");

    cout << "target_file (headers): " << targetFile << endl;
    cout << "tag_file: " << challengeFile << endl;
    cout << "NUM_SAMPLES: " << idashParams->NUM_SAMPLES << endl;
    cout << "NUM_REGIONS: " << idashParams->NUM_REGIONS << endl;
    cout << "REGION_SIZE: " << idashParams->REGION_SIZE << endl;
    cout << "NUM_TARGET_POSITIONS: " << idashParams->NUM_OUTPUT_POSITIONS << endl;
    cout << "NUM_TAG_POSITIONS: " << idashParams->NUM_INPUT_POSITIONS << endl;

    return idashParams;
}

IdashKey *keygen_ph2(IdashParams *idashParams) {
    //TRLWE parameters
    idashParams->tlweParams = new_TLweParams(idashParams->N, idashParams->k, idashParams->alpha, 0.25);

    TLweKey *tlweKey = new_TLweKey(idashParams->tlweParams);
    tLweKeyGen(tlweKey);
    IdashKey *key = new IdashKey(idashParams, tlweKey);

    return key;
}

void write_decrypted_predictions(const DecryptedPredictions &predictions, const IdashParams &params,
                                 const std::string &filename, const bool PRINT_POS_NAME) {
    ofstream out(filename);
    out << "Subject ID,target SNP,0,1,2" << endl;

    if (PRINT_POS_NAME) {
        for (uint64_t sampleId = 0; sampleId < params.NUM_SAMPLES; ++sampleId) {
            for (const auto &it : params.out_position_names) {
                const uint64_t &position = it.first;
                const std::string &position_name = it.second;
                const std::array<std::vector<float>, 3> &position_preds = predictions.score.at(position);
                out << sampleId << ",";
                out << position_name << ",";
                out << position_preds[0][sampleId] << ",";
                out << position_preds[1][sampleId] << ",";
                out << position_preds[2][sampleId] << endl;
            }
        }
    } else {
        for (uint64_t sampleId = 0; sampleId < params.NUM_SAMPLES; ++sampleId) {
            for (const auto &it : params.out_position_names) {
                const uint64_t &position = it.first;
                const std::array<std::vector<float>, 3> &position_preds = predictions.score.at(position);
                out << sampleId << ",";
                out << position << ",";
                out << position_preds[0][sampleId] << ",";
                out << position_preds[1][sampleId] << ",";
                out << position_preds[2][sampleId] << endl;
            }
        }
    }

    out.close();
}


/************************************************************************
****************************** KEY **************************************
************************************************************************/

void write_key(const IdashKey &key, const std::string &filename) {

    ofstream out(filename.c_str(), ios::binary);
    REQUIRE_DRAMATICALLY(out.is_open(), "Cannot open key file for write");


    write_params_ostream(*key.idashParams, out);
    export_tlweKey_toStream(out, key.tlweKey);

    out.close();
}

void read_key(IdashKey &key, const std::string &filename) {

    ifstream inp(filename.c_str(), ios::binary);
    REQUIRE_DRAMATICALLY(inp.is_open(), "Cannot open key file for read");

    IdashParams *params = new IdashParams();

    read_params_istream(*params, inp);
    key.idashParams = params;

    TLweKey *tlwekey = new_tlweKey_fromStream(inp);
    key.tlweKey = tlwekey;

    inp.close();
}





/************************************************************************
************************** ENCRYPTED DATA *******************************
************************************************************************/


void read_encrypted_data(EncryptedData &encrypted_data, const IdashParams &params, const std::string &filename)
{
    ifstream inp(filename);
    REQUIRE_DRAMATICALLY(inp, "Cannot open encrypted data file for read");

    // read the size of enc_data
    uint64_t length = encrypted_data.enc_data.size();
    istream_read_binary(inp, &length, sizeof(uint64_t));

    TLweSample *sample = new_TLweSample_array(length, params.tlweParams);

    // read the index and the tlwe sample
    for (uint64_t i = 0; i < length; ++i) {
        FeatIndex index;

        istream_read_binary(inp, &index, sizeof(FeatIndex));
        import_tlweSample_fromStream(inp, &sample[i], params.tlweParams);

        encrypted_data.enc_data.emplace(index, &sample[i]);
    }

    inp.close();
}




void write_encrypted_data(const EncryptedData &encrypted_data, const IdashParams &params, const std::string &filename)
{
    ofstream out(filename);
    REQUIRE_DRAMATICALLY(out, "Cannot open encrypted data file for write");

    // write the size of enc_data
    const uint64_t length = encrypted_data.enc_data.size();
    ostream_write_binary(out, &length, sizeof(uint64_t));

    // for each element of enc_data write the index and the tlwe sample, then export
    for (const auto &it : encrypted_data.enc_data) {
        ostream_write_binary(out, &it.first, sizeof(FeatIndex));
        export_tlweSample_toStream(out, it.second, params.tlweParams);
    }

    out.close();
}





/************************************************************************
********************** ENCRYPTED PREDICTIONS ****************************
************************************************************************/



void read_encrypted_predictions(EncryptedPredictions &encrypted_preds, const IdashParams &params,
                                const std::string &filename)
{
    ifstream inp(filename);
    REQUIRE_DRAMATICALLY(inp, "Cannot open encrypted predictions file for read");

    // read the size of enc_preds
    uint64_t length = encrypted_preds.score.size();
    istream_read_binary(inp, &length, sizeof(uint64_t));

    TLweSample *sample = new_TLweSample_array(length, params.tlweParams);


    // read the index and the tlwe sample
    for (uint64_t i = 0; i < length; ++i) {
        FeatBigIndex index;

        istream_read_binary(inp, &index, sizeof(FeatBigIndex));
        import_tlweSample_fromStream(inp, &sample[i], params.tlweParams);

        encrypted_preds.score.emplace(index, &sample[i]);
    }

    inp.close();
}



void write_encrypted_predictions(const EncryptedPredictions &encrypted_preds, const IdashParams &params,
                                 const std::string &filename)
{
    ofstream out(filename);
    REQUIRE_DRAMATICALLY(out, "Cannot open encrypted predictions file for write");

    // write the size of enc_preds
    const uint64_t length = encrypted_preds.score.size();
    ostream_write_binary(out, &length, sizeof(uint64_t));

    // for each element of enc_preds write the index and the tlwe sample, then export
    for (const auto &it : encrypted_preds.score) {
        ostream_write_binary(out, &it.first, sizeof(FeatBigIndex));
        export_tlweSample_toStream(out, it.second, params.tlweParams);
    }

    out.close();
}

void encrypt_data_ph1(EncryptedData &enc_data, const PlaintextData &plain_data, const IdashKey &key) {
    const IdashParams &params = *key.idashParams;
    const uint64_t NUM_SAMPLES = params.NUM_SAMPLES;
    REQUIRE_DRAMATICALLY(plain_data.data.size() == params.NUM_INPUT_POSITIONS, "Incomplete plaintext");
    //fill enc_data with ciphertexts of zero
    //pre-encrypt a pool of ciphertexts of zero
    const uint64_t NUM_CIPHERTEXTS = (params.NUM_INPUT_FEATURES+params.NUM_REGIONS-1)/params.NUM_REGIONS;
    TLweSample* pool = new_TLweSample_array(NUM_CIPHERTEXTS, params.tlweParams);
    #pragma omp parallel for num_threads(NB_THREADS)
    for (uint64_t i=0; i<NUM_CIPHERTEXTS; ++i) {
        tLweSymEncryptZero(pool+i, params.alpha, key.tlweKey);
    }


    TLweSample* nextSample = pool;
    for (const auto &it: plain_data.data) {
        const uint64_t &pos = it.first;
        const std::vector<int8_t> &values = it.second;
        REQUIRE_DRAMATICALLY(values.size() == NUM_SAMPLES, "plaintext dimensions inconsistency");
        enc_data.ensure_exists(params.inBigIdx(pos, 0), nextSample, params);
        enc_data.ensure_exists(params.inBigIdx(pos, 1), nextSample, params);
        enc_data.ensure_exists(params.inBigIdx(pos, 2), nextSample, params);
    }
    REQUIRE_DRAMATICALLY(nextSample<=pool+NUM_CIPHERTEXTS, "memory error");
    REQUIRE_DRAMATICALLY(nextSample>=pool+NUM_CIPHERTEXTS-10, "memory pb");
}

void encrypt_data_ph2(EncryptedData &enc_data, const PlaintextData &plain_data, const IdashKey &key) {
    const IdashParams &params = *key.idashParams;
    const uint64_t NUM_SAMPLES = params.NUM_SAMPLES;
    REQUIRE_DRAMATICALLY(plain_data.data.size() == params.NUM_INPUT_POSITIONS, "Incomplete plaintext");

    vector<pair<uint64_t, vector<int8_t>>> data_cpy(plain_data.data.begin(), plain_data.data.end());

    //add the actual scores
    #pragma omp parallel for num_threads(NB_THREADS)
    for (uint64_t i=0; i<data_cpy.size(); ++i) {
        auto &it = data_cpy[i];

        const uint64_t &pos = it.first;
        const std::vector<int8_t> &values = it.second;
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

void
decrypt_predictions(DecryptedPredictions &predictions, const EncryptedPredictions &enc_preds, const IdashKey &key) {

    const IdashParams &params = *key.idashParams;
    const uint64_t NUM_SAMPLES = params.NUM_SAMPLES;


    // initialize output
    for (const auto &it : params.out_features_index) {
        const uint64_t &outPos = it.first;

        for (int64_t snp = 0; snp < params.NUM_SNP_PER_POSITIONS; snp++) { // for snp in 0,1,2
            std::vector<float> &res = predictions.score[outPos][snp];  // this will create an empty vector in the result
            res.resize(NUM_SAMPLES);
        }
    }

    vector<pair<uint64_t, array<FeatBigIndex, 3>>> data_cpy(params.out_features_index.begin(), params.out_features_index.end());

    //add the actual scores
    #pragma omp parallel num_threads(NB_THREADS)
    {
        TorusPolynomial *plain = new_TorusPolynomial(params.N);

        #pragma omp for
        for (uint64_t i=0; i<data_cpy.size(); ++i) {
            auto &it = data_cpy[i];

        // for (const auto &it : params.out_features_index) {
            const uint64_t &outPos = it.first;
            for (int64_t snp = 0; snp < params.NUM_SNP_PER_POSITIONS; snp++) { // for snp in 0,1,2
                FeatBigIndex outBIdx = it.second[snp];
                const TLweSample *cipher = enc_preds.score.at(outBIdx);
                tLwePhase(plain, cipher, key.tlweKey); // -> a rescaler
                // rescale and copy result
                std::vector<float> &res = predictions.score[outPos][snp];
                for (uint64_t sample = 0; sample < NUM_SAMPLES; ++sample) {
                    res[sample] = t32tod(plain->coefsT[sample]);
                }
            }
            /*
            // renormalize all probabilities
            for (uint64_t sample = 0; sample < NUM_SAMPLES; ++sample) {
                double x0 = max<double>(0, predictions.score[outPos][0][sample]);
                double x1 = max<double>(0, predictions.score[outPos][1][sample]);
                double x2 = max<double>(0, predictions.score[outPos][2][sample]);
                double xNorm = x0 + x1 + x2;
                if (xNorm > 0) {
                    // normalize so that the sum is +1
                    predictions.score[outPos][0][sample] = x0 / xNorm;
                    predictions.score[outPos][1][sample] = x1 / xNorm;
                    predictions.score[outPos][2][sample] = x2 / xNorm;
                } else {
                    //consider these probas as NAN
                    predictions.score[outPos][0][sample] = 3. / 6.;
                    predictions.score[outPos][1][sample] = 2. / 6.;
                    predictions.score[outPos][2][sample] = 1. / 6.;
                }
            }
             */
        }

        // DELETE
        delete_TorusPolynomial(plain);
    }

/*
    for (const auto &it : enc_preds.score) {

        FeatBigIndex idx = it.first;
        predictions.score.first = toString(idx);

        TLweSample *cipher = it.second;

        tLweSymDecrypt(plain, cipher, key.tlweKey, Msize);

        //TODO how to retrieve the 3 probabilities?
    }
*/

}

void cloud_compute_score(EncryptedPredictions &enc_preds, const EncryptedData &enc_data,
                         const Model &model, const IdashParams &params) {
    // ============== apply model over ciphertexts
    const TLweParams *tlweParams = params.tlweParams;
    const int32_t k = tlweParams->k;
    REQUIRE_DRAMATICALLY(k == 1, "blah");
    const uint32_t N = params.N;
    const int64_t REGION_SIZE = params.REGION_SIZE;

    vector<pair<FeatBigIndex, unordered_map<FeatBigIndex, int32_t>>> model_vec(model.model.begin(), model.model.end());

    // create output tlwe beforehand
    for (const auto& it: model_vec) {
        enc_preds.createAndGet(it.first, tlweParams);
    }

    #pragma omp parallel num_threads(NB_THREADS)
    {
        // We use two temporary ciphertexts, one for region 0 and one for region 1
        // Only at the end of the loop we rotate region 1 and add it to region 0
        TLweSample *tmp = new_TLweSample_array(params.NUM_REGIONS, tlweParams);

        // create a temporary value to register the rotations
        TLweSample *tmp_rot = new_TLweSample(tlweParams);

        // for each output feature
        #pragma omp for
        for (unsigned i = 0; i < model_vec.size(); ++i) {
            FeatBigIndex outBidx = model_vec[i].first;
            const std::unordered_map<FeatBigIndex, int32_t> &mcoeffs = model_vec[i].second;

            //clear tmp
            for (uint64_t region = 0; region < params.NUM_REGIONS; ++region) {
                tLweClear(tmp + region, tlweParams);
            }

            //for each input feature, add it to the corresponding region
            for (const auto &it2: mcoeffs) {

                FeatBigIndex inBidx = it2.first;
                int32_t coeff = it2.second;

                if (inBidx == params.constant_bigIndex()) {
                    //add the constant to all the samples (implicitly) in region 0
                    Torus32 scaled_constant = coeff * params.ONE_IN_T32;
                    for (uint64_t sampleId = 0; sampleId < params.NUM_SAMPLES; ++sampleId) {
                        tmp->b->coefsT[sampleId] += scaled_constant;
                    }
                } else {
                    //add the TLWE to the corresponding region
                    FeatRegion region = params.feature_regionOf(inBidx);
                    const TLweSample *inTLWE = enc_data.getTLWE(inBidx, params);

                    // Multiply the scaled coefficient to the input and add it to the temporary region
                    tLweAddMulTo(tmp + region, coeff, inTLWE, tlweParams);
                }
            }

            // add all regions (rotated) to the output tlw
            TLweSample *outTLWE = enc_preds.get(outBidx, tlweParams);

            // Init with tmp region 0
            tLweCopy(outTLWE, tmp, tlweParams);
            for (uint64_t region = 1; region < params.NUM_REGIONS; ++region) {

                // in TFHE only tLweMulByXaiMinusOne is created, not tLweMulByXai
                // rotate the tmp regions
                int32_t rotation_amount = 2 * N - region * REGION_SIZE;
                for (int32_t i = 0; i <= k; i++) {
                    torusPolynomialMulByXai(&tmp_rot->a[i], rotation_amount, &(&tmp[region])->a[i]);
                }
                // add the rotation to outTLWE
                tLweAddTo(outTLWE, tmp_rot, tlweParams);
            }

            //destroy the positions that must remain hidden
            for (uint64_t j = params.REGION_SIZE; j < N; ++j) {
                outTLWE->b->coefsT[j] = 0;
            }
        }

        // DELETE
        delete_TLweSample(tmp_rot);
        delete_TLweSample_array(params.NUM_REGIONS, tmp);
    }
}

PlaintextOnehot compute_plaintext_onehot(const PlaintextData &X, const IdashParams &params) {
    // ============== apply model over plaintext
    //plaintext one hot encoded
    std::map<FeatBigIndex, std::vector<double>> plaintext_onehot;
    for (const auto &it: params.in_features_index) {
        const uint64_t &inPos = it.first;
        for (int inSNP = 0; inSNP < 3; inSNP++) {
            const FeatBigIndex inBIdx = it.second[inSNP];
            plaintext_onehot[inBIdx].resize(params.NUM_SAMPLES);
        }
        for (uint64_t sampleId = 0; sampleId < params.NUM_SAMPLES; ++sampleId) {
            switch (X.data.at(inPos)[sampleId]) {
                case 0:
                    plaintext_onehot[it.second[0]][sampleId] = params.ONE_IN_D;
                    plaintext_onehot[it.second[1]][sampleId] = 0;
                    plaintext_onehot[it.second[2]][sampleId] = 0;
                    break;
                case 1:
                    plaintext_onehot[it.second[0]][sampleId] = 0;
                    plaintext_onehot[it.second[1]][sampleId] = params.ONE_IN_D;
                    plaintext_onehot[it.second[2]][sampleId] = 0;
                    break;
                case 2:
                    plaintext_onehot[it.second[0]][sampleId] = 0;
                    plaintext_onehot[it.second[1]][sampleId] = 0;
                    plaintext_onehot[it.second[2]][sampleId] = params.ONE_IN_D;
                    break;
                default: //NAN
                    plaintext_onehot[it.second[0]][sampleId] = params.NAN_0_IN_D;
                    plaintext_onehot[it.second[1]][sampleId] = params.NAN_1_IN_D;
                    plaintext_onehot[it.second[2]][sampleId] = params.NAN_2_IN_D;
                    break;
            }
        }
    }
    return plaintext_onehot;
}

void compute_score(DecryptedPredictions &predictions, const PlaintextData &X,
                   const Model &M, const IdashParams &params) {
    // ============== apply model over plaintext

    //plaintext one hot encoded
    std::map<FeatBigIndex, std::vector<double>> plaintext_onehot = compute_plaintext_onehot(X, params);

    for (const auto &it: params.out_features_index) {
        const uint64_t &outPos = it.first;
        for (int outSNP = 0; outSNP < 3; outSNP++) {
            const FeatBigIndex outBIdx = it.second[outSNP];
            predictions.score[outPos][outSNP].clear();
            predictions.score[outPos][outSNP].resize(params.NUM_SAMPLES, 0);
            auto &coeffs = M.model.at(outBIdx); //coefficients for this output
            for (const auto &it2: coeffs) {
                for (uint64_t sampleId = 0; sampleId < params.NUM_SAMPLES; ++sampleId) {
                    //todo see what to do with the constant
                    if (it2.first == params.constant_bigIndex()) {
                        predictions.score[outPos][outSNP][sampleId] +=
                                it2.second * IdashParams::ONE_IN_D;
                    } else {
                        predictions.score[outPos][outSNP][sampleId] +=
                                it2.second * plaintext_onehot[it2.first][sampleId];
                    }
                }
            }
        }
        // //normalize preds
        // double pred[3];
        // for (uint64_t sampleId = 0; sampleId < params.NUM_SAMPLES; ++sampleId) {
        //     pred[0] = std::max<double>(0, predictions.score[outPos][0][sampleId]);
        //     pred[1] = std::max<double>(0, predictions.score[outPos][1][sampleId]);
        //     pred[2] = std::max<double>(0, predictions.score[outPos][2][sampleId]);
        //     double norm = pred[0] + pred[1] + pred[2];
        //     predictions.score[outPos][0][sampleId] = pred[0] / norm;
        //     predictions.score[outPos][1][sampleId] = pred[1] / norm;
        //     predictions.score[outPos][2][sampleId] = pred[2] / norm;
        // }
    }
}

// ---------- profiler class ---------------------------
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>


Profiler::Profiler() : tw0(Profiler::universalWallTime()), tc0(Profiler::universalClockTime()) {}

/* static */ double Profiler::universalWallTime() {
    struct timeval time;

    if (gettimeofday(&time, nullptr))
        return 0;

    return (double) time.tv_sec + (double) time.tv_usec * 1E-6;
}

/* static */ double Profiler::universalClockTime() {
    return (double) clock() / CLOCKS_PER_SEC;
}

double Profiler::walltime() const { return universalWallTime() - tw0; }
double Profiler::clocktime() const { return universalClockTime() - tc0; }

long int Profiler::maxrss() const {
    struct rusage usage;

    if (!getrusage(RUSAGE_SELF, &usage)) {
#ifndef __APPLE__
        return usage.ru_maxrss * 1000;
#else
        return usage.ru_maxrss;
#endif
    } else {
        return -1;
    }
}


