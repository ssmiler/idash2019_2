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

    std::string challengeFile = CHALLENGE_FILE;
    if (argc>=2) challengeFile = argv[1];
    std::cout << "using tag file (challenge): " << challengeFile << std::endl;

    IdashKey key;
    PlaintextData plain_data;
    EncryptedData enc_data;
    read_key(key, KEYS_FILE);
    read_plaintext_data(plain_data, *key.idashParams, challengeFile);
    const double time_start_encrypt = profiler.walltime();
    encrypt_data_ph1(enc_data, plain_data, key);
    encrypt_data_ph2(enc_data, plain_data, key);
    const double time_end_encrypt = profiler.walltime();
    write_encrypted_data(enc_data, *key.idashParams, ENCRYPTED_DATA_FILE);
    const double time_write_end = profiler.walltime();

    std::cout << "----------------- BENCHMARK ----------------- " << std::endl;
    std::cout << "encrypt wall time (seconds)......: " <<  time_end_encrypt-time_start_encrypt << std::endl;
    std::cout << "serialization wall time (seconds): " <<  time_write_end - time_end_encrypt + time_start_encrypt << std::endl;
    std::cout << "total wall time (seconds)........: " <<  time_write_end << std::endl;
    std::cout << "RAM usage (MB)...................: " <<  profiler.maxrss() / 1e6 << std::endl;
}
