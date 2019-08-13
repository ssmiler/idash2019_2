#include "idash.h"
#include <fstream>
#include <sstream>


int main() {
    const std::string targetFile;
    const std::string challengeFile;


    IdashKey *key = keygen(targetFile, challengeFile);
    write_params(*key->idashParams, "params_filename");
    write_key(*key, "key_filename");
}
