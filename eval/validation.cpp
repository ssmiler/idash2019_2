#include "idash.h"


/** data encryption test */
int main2() {
    PlaintextData data;
    EncryptedData encryptedData;

    IdashKey *key = keygen_ph2(keygen_ph1(TARGET_FILE, CHALLENGE_FILE, TARGET_FILE_POS_ONLY));
    const IdashParams &params = *key->idashParams;
    read_plaintext_data(data, params, CHALLENGE_FILE);

    PlaintextOnehot plaintextOnehot = compute_plaintext_onehot(data, params);
    encrypt_data_ph1(encryptedData, data, *key);
    encrypt_data_ph2(encryptedData, data, *key);

    //try to decrypt the ciphertext and compare to the plaintextOneHot
    TorusPolynomial *message = new_TorusPolynomial(params.N);
    double max_distance = -1;
    for (const auto &it: encryptedData.enc_data) {
        const FeatIndex idx = it.first;
        const TLweSample *const sample = it.second;
        tLwePhase(message, sample, key->tlweKey);
        for (uint64_t region = 0; region < params.NUM_REGIONS; ++region) {
            const FeatBigIndex bidx = params.feature_bigIndexOf(idx, region);
            if (plaintextOnehot.count(bidx) != 0) {
                //verify the decryption
                const std::vector<double> &actualPlaintext = plaintextOnehot.at(bidx);
                for (uint64_t sampleId = 0; sampleId < params.NUM_SAMPLES; ++sampleId) {
                    double mess = t32tod(message->coefsT[sampleId + region * params.REGION_SIZE]);
                    double expected = actualPlaintext[sampleId];
                    REQUIRE_DRAMATICALLY(fabs(mess - expected) < 0.01, "bug");
                    max_distance = std::max<double>(max_distance, fabs(mess - expected));
                }
            }
        }
    }
    delete_TorusPolynomial(message);
    std::cout << "maximal distance in ciphertext: " << max_distance << std::endl;
    return 0;
}

/** cloud compute test */
int main() {
    Model model;
    PlaintextData data;
    DecryptedPredictions targetPredictions;
    EncryptedData encryptedData;
    DecryptedPredictions resultPredications;
    EncryptedPredictions encPredications;

    IdashKey *key = keygen_ph2(keygen_ph1(TARGET_FILE, CHALLENGE_FILE, TARGET_FILE_POS_ONLY));
    const IdashParams &params = *key->idashParams;
    read_model(model, params, "../../ml/model/hr/10k");
    read_plaintext_data(data, params, CHALLENGE_FILE);

    compute_score(targetPredictions, data, model, params);

    encrypt_data_ph1(encryptedData, data, *key);
    encrypt_data_ph2(encryptedData, data, *key);
    cloud_compute_score(encPredications, encryptedData, model, params);
    decrypt_predictions(resultPredications, encPredications, *key);


    DecryptedPredictions::testEquals(targetPredictions, resultPredications, params);
    write_decrypted_predictions(resultPredications, params, "pos_preds", false);
    write_decrypted_predictions(resultPredications, params, "named_preds", true);
    return 0;
}
