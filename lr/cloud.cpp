#include "idash.h"

void cloud_compute_score(EncryptedPredictions &enc_preds, const EncryptedData &enc_data,
                         const Model &model, const IdashParams &params) {
    // ============== apply model over ciphertexts
    const TLweParams *tlweParams = params.tlweParams;
    const int32_t k = tlweParams->k;
    REQUIRE_DRAMATICALLY(k == 1, "blah");
    const uint32_t N = params.N;
    const int64_t REGION_SIZE = params.REGION_SIZE;


    // We use two temporary ciphertexts, one for region 0 and one for region 1
    // Only at the end of the loop we rotate region 1 and add it to region 0
    TLweSample *tmp = new_TLweSample_array(params.NUM_REGIONS, tlweParams);

    // create a temporary value to register the rotations
    TLweSample *tmp_rot = new_TLweSample(tlweParams);


    // for each output feature
    for (const auto &it : model.model) {

        FeatBigIndex outBidx = it.first;
        const std::unordered_map<FeatBigIndex, int32_t> &mcoeffs = it.second;

        //clear tmp
        for (uint64_t region = 0; region < params.NUM_REGIONS; ++region) {
            tLweClear(tmp + region, tlweParams);
        }

        //for each input feature, add it to the corresponding region
        for (const auto &it2: mcoeffs) {

            FeatBigIndex inBidx = it2.first;
            int32_t coeff = it2.second;

            FeatRegion region = params.feature_regionOf(inBidx);
            const TLweSample *inTLWE = enc_data.getTLWE(inBidx, params);

            // Multiply the scaled coefficient to the input and add it to the temporary region
            tLweAddMulTo(tmp + region, coeff, inTLWE, tlweParams);
        }

        // add all regions (rotated) to the output tlw
        TLweSample *outTLWE = enc_preds.createAndGet(outBidx, tlweParams);

        // Init with tmp region 0
        tLweCopy(outTLWE, tmp, tlweParams);
        for (uint64_t region = 1; region < params.NUM_REGIONS; ++region) {

            // in TFHE only tLweMulByXaiMinusOne is created, not tLweMulByXai
            // rotate the tmp regions
            for (int32_t i = 0; i <= k; i++){
                torusPolynomialMulByXai(&tmp_rot->a[i], -region * REGION_SIZE, &tmp[region].a[i]);
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

int main() {
    IdashParams params;
    Model model;
    EncryptedData enc_data;
    EncryptedPredictions enc_preds;
    read_params(params, "params_file");
    read_model(model, params, "../ml/model/hr/10k");
    read_encrypted_data(enc_data, params, "enc_data_file");
    cloud_compute_score(enc_preds, enc_data, model, params);
    write_encrypted_predictions(enc_preds, params, "enc_preds_filename");
}

