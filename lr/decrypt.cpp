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

int main(int argc, char** argv) {
    Profiler profiler;
    IdashKey key;
    EncryptedPredictions enc_predictions;
    DecryptedPredictions dec_predictions;

    read_key(key, KEYS_FILE);
    read_encrypted_predictions(enc_predictions, *key.idashParams, ENCRYPTED_PREDICTION_FILE);
    const double time_decrypt_start = profiler.walltime();
    decrypt_predictions(dec_predictions, enc_predictions, key);
    const double time_decrypt_end = profiler.walltime();
    if (argc==1) {
        write_decrypted_predictions(dec_predictions, *key.idashParams, RESULT_FILE, 1);
    } else {
        write_decrypted_predictions(dec_predictions, *key.idashParams, RESULT_BYPOS_FILE, 0);
    }

    /** just to test if the result is the same as validate2
    for (uint64_t sampleId = 0; sampleId < key.idashParams->NUM_SAMPLES; ++sampleId) {
        for (const auto &it : key.idashParams->out_features_index) {
            const uint64_t &pos = it.first;
                std::cout << dec_predictions.score.at(pos)[0][sampleId] << std::endl;
                std::cout << dec_predictions.score.at(pos)[1][sampleId] << std::endl;
                std::cout << dec_predictions.score.at(pos)[2][sampleId] << std::endl;
        };
    }
     */

    const double time_write_end = profiler.walltime();

    std::cout << "----------------- BENCHMARK ----------------- " << std::endl;
    std::cout << "decrypt wall time (seconds)......: " <<  time_decrypt_end-time_decrypt_start << std::endl;
    std::cout << "serialization wall time (seconds): " <<  time_write_end - time_decrypt_end + time_decrypt_start << std::endl;
    std::cout << "total wall time (seconds)........: " <<  time_write_end << std::endl;
    std::cout << "RAM usage (MB)...................: " <<  profiler.maxrss() / 1e6 << std::endl;

}
