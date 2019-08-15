#include "idash.h"



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

