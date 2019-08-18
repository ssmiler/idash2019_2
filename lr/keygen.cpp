#include "idash.h"

int main(int argc, char** argv) {

    const std::string targetFile = TARGET_FILE;
    const std::string challengeFile = CHALLENGE_FILE;

    IdashKey *key = keygen(targetFile, challengeFile);
    write_params(*key->idashParams, PARAMS_FILE);
    write_key(*key, KEYS_FILE);
}
