#include "idash.h"


int main() {
    Model model;
    PlaintextData data;
    DecryptedPredictions predictions;

    IdashKey *key = keygen(TARGET_FILE, CHALLENGE_FILE, TARGET_FILE_POS_ONLY);
    const IdashParams &params = *key->idashParams;
    read_model(model, params, "../../ml/model/hr/10k");
    read_plaintext_data(data, params, CHALLENGE_FILE);
    compute_score(predictions, data, model, params);
    write_decrypted_predictions(predictions, params, "preds_filename", 0);
}

