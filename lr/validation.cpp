#include "idash.h"

int main() {
  Model model;
  PlaintextData data;
  DecryptedPredictions targerPredictions;
  EncryptedData encryptedData;
  DecryptedPredictions resultPredications;
  EncryptedPredictions encPredications;

  IdashKey *key = keygen(TARGET_FILE, CHALLENGE_FILE);
  const IdashParams &params = *key->idashParams;
  read_model(model, params, "../../ml/model/hr/10k");
  read_plaintext_data(data, params, CHALLENGE_FILE);

  compute_score(targerPredictions, data, model, params);
  write_decrypted_predictions(targerPredictions, params, "target_preds_filename");

  encrypt_data(encryptedData, data, *key);
  cloud_compute_score(encPredications, encryptedData, model, params);
  decrypt_predictions(resultPredications, encPredications, *key);
  write_decrypted_predictions(targerPredictions, params, "res_preds_filename");
}
