#include "idash.h"

void cloud_compute_score(EncryptedPredictions &enc_preds, const EncryptedData &enc_data,
                            const Model &model, const IdashParams &params,
                            const TLweParams &tlweParams) {
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
    const int32_t k = tlweParams.k;
    const uint32_t N = params.N;
    const uint32_t N2 = params.N / 2;


    // We use two temporary ciphertexts, one for region 0 and one for region 1
    // Only at the end of the loop we rotate region 1 and add it to region 0
    TLweSample *tmp = new_TLweSample_array(params.NUM_REGIONS, tlweParams);
    // ILA: params.NUM_REGIONS should be defined as int32_t instead of uint32_t
    // ILA: not sure tlwe.h was included in tfhe.h, that might be the reason of the error?

    // create a temporary value to register the rotations
    TLweSample *tmp_rot = new_TLweSample(tlweParams);
    // create the random polynomial
    TorusPolynomial *rand_poly = new_TorusPolynomial(N*params.NUM_REGIONS);


    // for each output feature
    for (const auto &it : model.model) {

        FeatBigIndex outBidx = it.first;
        const std::unordered_map<FeatBigIndex, float> &mcoeffs = it.second;

        //clear tmp
        for (uint64_t region = 0; region < params.NUM_REGIONS; ++region) {
            tLweClear(tmp + region, tlweParams);
            // ILA: not sure tlwe_functions.h was included in tfhe.h, that might be the reason of the error?
        }

        //for each input feature, add it to the corresponding region
        for (const auto &it2: mcoeffs) {

            FeatBigIndex inBidx = it2.first;
            float coeff = it2.second;

            FeatRegion region = params.feature_regionOf(inBidx);
            const TLweSample *inTLWE = enc_data.getTLWE(inBidx);

            // rescale the model coefficient
            int32_t scaled_coeff = int32_t(rint(coeff * COEFF_SCALING_FACTOR));
            // Multiply the scaled coefficient to the input and add it to the temporary region
            tLweAddMulTo(tmp + region, scaled_coeff, inTLWE, tlweParams);
        }

        // add all regions (rotated) to the output tlwe
        TLweSample *outTLWE = enc_preds.createAndGet(outBidx);

        // Init with tmp region 0
        outTLWE = tmp;
        for (uint64_t region = 1; region < params.NUM_REGIONS; ++region) {

            // in TFHE only tLweMulByXaiMinusOne is created, not tLweMulByXai
            // rotate the tmp regions
            for (int32_t i = 0; i <= k; i++){
                torusPolynomialMulByXai(&tmp_rot->a[i], -region*N2, &tmp[region].a[i]);
            }
            // add the rotation to outTLWE
            tLweAddTo(outTLWE, tmp_rot, tlweParams);
        }

        //randomize the positions that must remain hidden
        // initialize the random polynomial at 0
        torusPolynomialClear(rand_poly);
        // randomize the coefficients after N
        for (int32_t j = N; j < N*params.NUM_REGIONS; ++j) {
            // ILA: which random function do we want to use?
            rand_poly[j] = rand(); //
        }
        torusPolynomialAddTo(&outTLWE->b, &rand_poly);

        // ILA: We should register this in enc_preds???
    }

    // DELETE
    delete_TorusPolynomial(rand_poly);
    delete_TLweSample(tmp_rot);
    delete_TLweSample_array(params.NUM_REGIONS, tmp);
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

