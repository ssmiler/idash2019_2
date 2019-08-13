#include "idash.h"
#include "parse_vw.h"

#include <iostream>
#include <fstream>

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
pair<string, uint8_t> split(const string& buf) {
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
  for (const auto & elem: params.out_features_index) {
    string pos = elem.first;
    auto out_bindices = elem.second;

    for (int snp = 0; snp < 3; ++snp)
    {
      string file_name = path + "/" + pos + "_" + to_string(snp) + ".hr";
      unordered_map<string, int32_t> coefs = read(file_name);

      FeatBigIndex out_bidx = out_bindices[snp];
      auto& model_coefs = model.model[out_bidx] = unordered_map<FeatBigIndex, int32_t>();

      // transform raw coefficients to BigIndex
      for (const auto& elem : coefs) {
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

void write_params(const IdashParams &params, const string &filename) {
  ofstream out(filename.c_str(), ios::binary);

  REQUIRE_DRAMATICALLY(out.is_open(), "Cannot open parameters file for write")

  out.write((char *)(&params.NUM_SAMPLES), sizeof(params.NUM_SAMPLES));
  out.write((char *)(&params.NUM_INPUT_POSITIONS),
            sizeof(params.NUM_INPUT_POSITIONS));
  out.write((char *)(&params.NUM_OUTPUT_POSITIONS),
            sizeof(params.NUM_OUTPUT_POSITIONS));
  out.write((char *)(&params.NUM_INPUT_FEATURES),
            sizeof(params.NUM_INPUT_FEATURES));
  out.write((char *)(&params.NUM_OUTPUT_FEATURES),
            sizeof(params.NUM_OUTPUT_FEATURES));

  REQUIRE_DRAMATICALLY(params.NUM_INPUT_POSITIONS ==
                           params.in_features_index.size(),
                       "NUM_INPUT_POSITIONS != in_features_index.size()")

  for (const auto &e1 : params.in_features_index) {
    string pos = e1.first;

    // write feature position
    size_t pos_size = pos.size();
    out.write((char *)&pos_size, sizeof(size_t));
    out.write(pos.c_str(), sizeof(char) * pos.size());

    // write snps
    for (const FeatBigIndex e2 : e1.second) {
      out.write((char *)(&e2), sizeof(e2));
    }
  }

  REQUIRE_DRAMATICALLY(params.NUM_OUTPUT_POSITIONS ==
                           params.out_features_index.size(),
                       "NUM_OUTPUT_POSITIONS != out_features_index.size()")

  for (const auto &e1 : params.out_features_index) {
    string pos = e1.first;

    // write feature position
    size_t pos_size = pos.size();
    out.write((char *)&pos_size, sizeof(size_t));
    out.write(pos.c_str(), sizeof(char) * pos.size());

    // write snps
    for (const FeatBigIndex e2 : e1.second) {
      out.write((char *)(&e2), sizeof(e2));
    }
  }

  out.close();
}

void read_params(IdashParams &params, const string &filename) {
  ifstream inp(filename.c_str(), ios::binary);

  REQUIRE_DRAMATICALLY(inp.is_open(), "Cannot open parameters file for read")

  inp.read((char *)(&params.NUM_SAMPLES), sizeof(params.NUM_SAMPLES));
  inp.read((char *)(&params.NUM_INPUT_POSITIONS),
           sizeof(params.NUM_INPUT_POSITIONS));
  inp.read((char *)(&params.NUM_OUTPUT_POSITIONS),
           sizeof(params.NUM_OUTPUT_POSITIONS));
  inp.read((char *)(&params.NUM_INPUT_FEATURES),
           sizeof(params.NUM_INPUT_FEATURES));
  inp.read((char *)(&params.NUM_OUTPUT_FEATURES),
           sizeof(params.NUM_OUTPUT_FEATURES));

  char buff[256];

  for (uint32_t i = 0; i < params.NUM_INPUT_POSITIONS; ++i) {
    size_t pos_size;

    // read feature position
    inp.read((char *)&pos_size, sizeof(size_t));
    REQUIRE_DRAMATICALLY(pos_size < 255, "buffer overflow");
    inp.read(buff, pos_size);
    buff[pos_size + 1] = '\0';

    // read snps
    params.in_features_index.emplace(buff, array<FeatBigIndex, 3>());
    auto &tmp = params.in_features_index.at(buff);
    for (int j = 0; j < 3; ++j) {
      inp.read((char *)(&tmp[j]), sizeof(FeatBigIndex));
    }
  }

  REQUIRE_DRAMATICALLY(params.NUM_INPUT_POSITIONS ==
                           params.in_features_index.size(),
                       "NUM_INPUT_POSITIONS != in_features_index.size()")

  for (uint32_t i = 0; i < params.NUM_OUTPUT_POSITIONS; ++i) {
    size_t pos_size;

    // read feature position
    inp.read((char *)&pos_size, sizeof(size_t));
    REQUIRE_DRAMATICALLY(pos_size < 255, "buffer overflow");
    inp.read(buff, pos_size);
    buff[pos_size + 1] = '\0';

    // read snps
    params.out_features_index.emplace(buff, array<FeatBigIndex, 3>());
    auto &tmp = params.out_features_index.at(buff);
    for (int j = 0; j < 3; ++j) {
      inp.read((char *)(&tmp[j]), sizeof(FeatBigIndex));
    }
  }

  REQUIRE_DRAMATICALLY(params.NUM_OUTPUT_POSITIONS ==
                           params.out_features_index.size(),
                       "NUM_OUTPUT_POSITIONS != out_features_index.size()")

  inp.close();
}
