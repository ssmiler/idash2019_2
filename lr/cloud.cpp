#include "idash.h"

void cloud_compute_score(EncryptedPredictions &enc_preds, const EncryptedData &enc_data, const Model &model,
                         const IdashParams &params) {
    /*
       std::vector <std::unordered_map<std::string, float>> model;
       read(model, "../15832228.model.hr");

       const uint64_t n = model.size();
       for (uint64_t i = 0; i < n; i++) {
           cout << i << endl;
           for (const auto &it: model[i]) {
               cout << it.first << " => " << it.second << endl;
           }
       }
   */
    // ============== apply model over ciphertexts
    const double &COEFF_SCALING_FACTOR = params.COEF_SCALING_FACTOR;
    TLweSample *tmp = new_TLweSample_array(params.NUM_REGIONS, tlweParams);

    // for each output feature
    for (const auto &it : model.model) {
        FeatBigIndex outBidx = it.first;
        const std::unordered_map<FeatBigIndex, float> &mcoeffs = it.second;
        //clear tmp
        for (uint64_t region = 0; region < params.NUM_REGIONS; ++region) {
            tLweClear(tmp + region, tlweParams);
        }
        //for each input feature, add it to the corresponding region
        for (const auto &it2: mcoeffs) {
            FeatBigIndex inBidx = it2.first;
            float coeff = it2.second;
            FeatRegion region = params.feature_regionOf(inBidx);
            const TLweSample *inTLWE = enc_data.getTLWE(inBidx);
            int32_t scaled_coeff = int32_t(rint(coeff * params.COEF_SCALING_FACTOR));
            tLweAddMulTo(tmp + region, scaled_coeff, inTLWE, tlweParams);
        }
        //add all regions (rotated) to the output tlwe
        TLweSample *outTLWE = enc_preds.createAndGet(outBidx);
        //TODO
        abort();
        //randomize the positions that must remain hidden
        //TODO
        abort();
    }

    delete_TLweSample_array(params.NUM_REGIONS, tlweParams);
}

int main() {
    IdashParams params;
    Model model;
    EncryptedData enc_data;
    EncryptedPredictions enc_preds;
    read_params(params, "params_file");
    read_model(model, params, "model_file");
    read_encrypted_data(enc_data, params, "enc_data_file");
    cloud_compute_score(enc_preds, enc_data, model, params);
    write_encrypted_predictions(enc_preds, params, "enc_preds_filename");
}

