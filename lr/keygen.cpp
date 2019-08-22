#include "idash.h"

int main(int argc, char** argv) {
    Profiler profiler;
    std::string targetFile = TARGET_FILE;
    std::string challengeFile = CHALLENGE_FILE;
    if (argc>=2) targetFile = argv[1];
    if (argc>=3) challengeFile = argv[2];
    std::cout << "using target file (headers): " << targetFile << std::endl;
    std::cout << "using tag file (challenge): " << challengeFile << std::endl;

    IdashKey *key = keygen(targetFile, challengeFile);
    write_params(*key->idashParams, PARAMS_FILE);
    write_key(*key, KEYS_FILE);

    std::cout << "BENCHMARK:" << std::endl;
    std::cout << "wall time (seconds): " <<  profiler.walltime() << std::endl;
    std::cout << "RAM (MB): " <<  profiler.maxrss()  / 1e6 << std::endl;
}
