#include "idash.h"

void compute_score(DecryptedPredictions &predictions, const PlaintextData &X,
                   const Model &M, const IdashParams &params) {
    // ============== apply model over plaintext

    //plaintext one hot encoded
    std::map<FeatBigIndex, std::vector<double>> plaintext_onehot;
    for (const auto &it: params.in_features_index) {
        const std::string &inPos = it.first;
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

    for (const auto &it: params.out_features_index) {
        const std::string &outPos = it.first;
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

int main() {
    Model model;
    PlaintextData data;
    DecryptedPredictions predictions;

    IdashKey *key = keygen(TARGET_FILE, CHALLENGE_FILE);
    const IdashParams &params = *key->idashParams;
    read_model(model, params, "../../ml/model/hr/10k");
    read_plaintext_data(data, params, CHALLENGE_FILE);
    compute_score(predictions, data, model, params);
    write_decrypted_predictions(predictions, params, "preds_filename");
}

