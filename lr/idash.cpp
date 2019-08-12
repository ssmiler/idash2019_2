#include "idash.h"
#include "parse_vw.h"

using namespace std;

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
