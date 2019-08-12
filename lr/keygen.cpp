#include "idash.h"
#include <fstream>
#include <sstream>

IdashKey *keygen(const std::string &targetFile, const std::string &challengeFile) {


    IdashParams *idashParams = new IdashParams();
    const TLweKey *tlweKey;

    std::string line;

    //read all output positions from the targetFile
    // initializes: NUM_OUTPUT_POSITIONS, NUM_OUTPUT_FEATURES, out_bidx_map
    std::ifstream target(targetFile);
    idashParams->NUM_OUTPUT_POSITIONS = 0;
    idashParams->NUM_OUTPUT_FEATURES = 0;
    for (std::getline(target, line); target; std::getline(target, line)) {
        std::istringstream iss(line);
        int blah = 0; // must be 1 in the file
        std::string position;
        iss >> blah;
        REQUIRE_DRAMATICALLY(blah == 1, "file format error");
        iss >> position;
        REQUIRE_DRAMATICALLY(iss, "file format error");
        idashParams->registerOutBigIdx(position, 0, idashParams->NUM_OUTPUT_FEATURES++);
        idashParams->registerOutBigIdx(position, 1, idashParams->NUM_OUTPUT_FEATURES++);
        idashParams->registerOutBigIdx(position, 2, idashParams->NUM_OUTPUT_FEATURES++);
        idashParams->NUM_OUTPUT_POSITIONS++;
    }
    target.close();

    //read all output positions from the challengeFile
    // initializes: NUM_OUTPUT_POSITIONS, NUM_OUTPUT_FEATURES, out_bidx_map
    std::ifstream challenge(challengeFile);
    idashParams->NUM_SAMPLES = 0;
    idashParams->NUM_INPUT_POSITIONS = 0;
    idashParams->NUM_INPUT_FEATURES = 0;
    for (std::getline(challenge, line); challenge; std::getline(challenge, line)) {
        std::istringstream iss(line);
        int blah = 0; // must be 1 in the file
        std::string position;
        iss >> blah;
        REQUIRE_DRAMATICALLY(blah == 1, "file format error");
        iss >> position;
        REQUIRE_DRAMATICALLY(iss, "file format error");
        idashParams->registerInBigIdx(position, 0, idashParams->NUM_INPUT_FEATURES++);
        idashParams->registerInBigIdx(position, 1, idashParams->NUM_INPUT_FEATURES++);
        idashParams->registerInBigIdx(position, 2, idashParams->NUM_INPUT_FEATURES++);
        idashParams->NUM_INPUT_POSITIONS++;
        if (idashParams->NUM_SAMPLES == 0) {
            std::string position2;
            std::string featureName;
            iss >> position2;
            iss >> featureName;
            REQUIRE_DRAMATICALLY(iss, "file format error");
            while (true) {
                uint8_t snp;
                iss >> snp;
                if (!iss) break;
                REQUIRE_DRAMATICALLY(snp == 0 || snp == 1 || snp == 2, "file format error");
                idashParams->NUM_SAMPLES++;
            }
        }
        idashParams->NUM_INPUT_FEATURES++; //for the constant value
    }
    challenge.close();


    //TRLWE parameters
    idashParams->tlweParams = new_TLweParams(idashParams->N, idashParams->k, idashParams->alpha, 0.25);


    REQUIRE_DRAMATICALLY(idashParams->REGION_SIZE >= idashParams->NUM_SAMPLES, "REGION_SIZE must be >= NUM_SAMPLES ");

    tlweKey = new_TLweKey(idashParams->tlweParams);
    IdashKey *key = new IdashKey(idashParams, tlweKey);
    return key;
}


int main() {
    const std::string targetFile;
    const std::string challengeFile;


    IdashKey *key = keygen(targetFile, challengeFile);
    write_params(*key->idashParams, "params_filename");
    write_key(*key, "key_filename");
}
