#include "idash.h"

/** cloud compute test */
int main() {
    Model model;
    PlaintextData data;
    DecryptedPredictions targetPredictions;
    EncryptedData encryptedData;
    DecryptedPredictions resultPredications;
    EncryptedPredictions encPredictions;


    IdashKey *key = keygen(TARGET_FILE, CHALLENGE_FILE, TARGET_FILE_POS_ONLY);
    write_key(*key, "file_key");

    IdashKey key_read;
    read_key(key_read, "file_key");

    //const IdashParams &params = *key_read.idashParams;
    write_params(*key_read.idashParams, "file_params");
    IdashParams params_read;
    read_params(params_read, "file_params");

    read_model(model, params_read, MODEL_FILE);
    read_plaintext_data(data, params_read, CHALLENGE_FILE);

    compute_score(targetPredictions, data, model, params_read);

    encrypt_data(encryptedData, data, key_read);

    write_encrypted_data(encryptedData, params_read, "file_encrypt_data");
    EncryptedData encryptedData_read;
    read_encrypted_data(encryptedData_read, params_read, "file_encrypt_data");

    cloud_compute_score(encPredictions, encryptedData_read, model, params_read);

    write_encrypted_predictions(encPredictions, params_read, "file_encrypt_prediction");
    EncryptedPredictions encPredictions_read;
    read_encrypted_predictions(encPredictions_read, params_read, "file_encrypt_prediction");

    decrypt_predictions(resultPredications, encPredictions_read, key_read);


    DecryptedPredictions::testEquals(targetPredictions, resultPredications, params_read);
    write_decrypted_predictions(resultPredications, params_read, "pos_preds2", false);
    write_decrypted_predictions(resultPredications, params_read, "named_preds2", true);
    return 0;
}
