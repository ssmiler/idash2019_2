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
    PlaintextData plain_data;
    EncryptedData enc_data;
    read_key(key, "key_file");
    read_plaintext_data(plain_data, *key.idashParams, "plain_data_file");
    encrypt_data(enc_data, plain_data, key);
    write_encrypted_data(enc_data, *key.idashParams, "enc_data_file");
}
