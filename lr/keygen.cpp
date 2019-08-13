#include "idash.h"
#include <fstream>
#include <sstream>

int main(int argc, char** argv) {
    if (argc < 3) {
        DIE_DRAMATICALLY("Please provide target and tag files")
    }

    const std::string targetFile = argv[1];
    const std::string challengeFile = argv[2];


    IdashKey *key = keygen(targetFile, challengeFile);
    write_params(*key->idashParams, "params_filename");
    write_key(*key, "key_filename");
}
