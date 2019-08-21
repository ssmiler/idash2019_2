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
    std::string challengeFile = CHALLENGE_FILE;
    if (argc>=2) challengeFile = argv[1];
    std::cout << "using tag file (challenge): " << challengeFile << std::endl;

    IdashKey key;
    PlaintextData plain_data;
    EncryptedData enc_data;
    read_key(key, KEYS_FILE);
    read_plaintext_data(plain_data, *key.idashParams, challengeFile);
    encrypt_data(enc_data, plain_data, key);
    write_encrypted_data(enc_data, *key.idashParams, ENCRYPTED_DATA_FILE);
}
