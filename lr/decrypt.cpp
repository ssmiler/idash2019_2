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
    const int32_t Msize; // TODO

    predictions = createAndGet(std::string
    model, *key.idashParams);

    TorusPolynomial *plain = new_TorusPolynomial(params.N);

    for (const auto &it : enc_preds.score) {

        FeatBigIndex idx = it.first;
        predictions.score.first = toString(idx);

        TLweSample *cipher = it.second;

        tLweSymDecrypt(plain, cipher, key.tlweKey, Msize);

        //TODO how to retrieve the 3 probabilities?
    }



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

