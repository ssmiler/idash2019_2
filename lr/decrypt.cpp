#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <iostream>
#include <cmath>
#include <tfhe.h>
#include "idash.h"
#include "parse_vw.h"

using namespace std;

//TODO shall we take in input enc_data or end_predictions?

void
decrypt_predictions(DecryptedPredictions &predictions, const EncryptedPredictions &enc_preds, const IdashKey &key) {

    const IdashParams &params = *key.idashParams;
    const uint64_t NUM_SAMPLES = params.NUM_SAMPLES;

    TorusPolynomial *plain = new_TorusPolynomial(params.N);

    for (const auto &it : params.out_features_index) {
        const std::string &outPos = it.first;
        for (int64_t snp = 0; snp < params.NUM_SNP_PER_POSITIONS; snp++) { // for snp in 0,1,2
            FeatBigIndex outBIdx = it.second[snp];
            const TLweSample *cipher = enc_preds.score.at(outBIdx);
            tLwePhase(plain, cipher, key.tlweKey); // -> a rescaler
            // initialize output
            std::vector<float> &res = predictions.score[outPos][snp];  // this will create an empty vector in the result
            res.resize(NUM_SAMPLES);
            // rescale and copy result
            for (uint64_t sample = 0; sample < NUM_SAMPLES; ++sample) {
                res[sample] = plain->coefsT[sample] * params.OUT_SCALING_FACTOR;
            }
        }
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


    // DELETE
    delete_TorusPolynomial(plain);
}

int main() {
    IdashKey key;
    EncryptedPredictions enc_predictions;
    DecryptedPredictions dec_predictions;

    read_key(key, "key_file");
    read_encrypted_predictions(enc_predictions, *key.idashParams, "enc_prediction_file");
    decrypt_predictions(dec_predictions, enc_predictions, key);
    write_decrypted_predictions(dec_predictions, *key.idashParams, "dec_prediction_file");
}

