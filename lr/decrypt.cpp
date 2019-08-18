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

int main() {
    IdashKey key;
    EncryptedPredictions enc_predictions;
    DecryptedPredictions dec_predictions;

    read_key(key, KEYS_FILE);
    read_encrypted_predictions(enc_predictions, *key.idashParams, ENCRYPTED_PREDICTION_FILE);
    decrypt_predictions(dec_predictions, enc_predictions, key);
    write_decrypted_predictions(dec_predictions, *key.idashParams, RESULT_FILE, 0);
    write_decrypted_predictions(dec_predictions, *key.idashParams, RESULT_BYPOS_FILE, 1);
}
