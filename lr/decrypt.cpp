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



int main() {
    IdashKey key;
    EncryptedPredictions enc_predictions;
    DecryptedPredictions dec_predictions;

    read_key(key, "key_file");
    read_encrypted_predictions(enc_predictions, *key.idashParams, "enc_prediction_file");
    decrypt_predictions(dec_predictions, enc_predictions, key);
    write_decrypted_predictions(dec_predictions, *key.idashParams, "dec_prediction_file", 0);
}

