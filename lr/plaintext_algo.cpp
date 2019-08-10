#include "idash.h"

void compute_score(DecryptedPredictions &predictions, const PlaintextData &X,
                   const Model &M, const IdashParams &params) {
    // ============== apply model over plaintext




    for (const auto &j: X.data) { //j
        const std::string &pos = j.first;
        const std::vector<int8_t> &valuesX = j.second;
        for (const auto &k : M.model) { //k
            FeatBigIndex outIndex = k.first;
            const std::unordered_map<FeatBigIndex, int32_t> &valeusM = k.second;
            predictions.score[outIndex] = 0; //to check Y[i][k]=:0
            for (uint64_t i = 0; i < NUM_SAMPLES; i++) {
                predictions.score[outIndex] += valuesX[i] * valeusM[pos];    //Y[i][k]=+X[i][j]*M[j][k]
            }
        }
    }
}

int main() {
    IdashParams params;
    Model model;
    PlaintextData data;
    DecryptedPredictions predictions;

    read_params(params, "params_file");
    read_model(model, params, "../ml/model/hr/10k");
    read_plaintext_data(data, params, "data_file");
    compute_score(predictions, data, model, params);
    write_decrypted_predictions(predictions, params, "preds_filename");
}

