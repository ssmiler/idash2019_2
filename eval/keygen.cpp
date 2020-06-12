#include "idash.h"

int main(int argc, char** argv) {
    Profiler profiler;
    std::string targetFile = TARGET_FILE;
    std::string challengeFile = CHALLENGE_FILE;
    bool targetFilePosOnly = TARGET_FILE_POS_ONLY;
    if (argc>=2) targetFile = argv[1];
    if (argc>=3) challengeFile = argv[2];
    if (argc>=4) targetFilePosOnly = (bool)atoi(argv[3]);
    std::cout << "using target file (headers): " << targetFile;
    if (targetFilePosOnly) {
        std::cout << " (only positions)";
    }
    std::cout << std::endl;
    std::cout << "using tag file (challenge): " << challengeFile << std::endl;

    IdashParams *params = keygen_ph1(targetFile, challengeFile, targetFilePosOnly);

    const double time_A = profiler.walltime();
    IdashKey *key = keygen_ph2(params);
    const double time_B = profiler.walltime();

    write_params(*key->idashParams, PARAMS_FILE);
    write_key(*key, KEYS_FILE);

    std::cout << "----------------- BENCHMARK ----------------- " << std::endl;
    std::cout << "Number of threads ...............: " <<  NB_THREADS << std::endl;
    std::cout << "Keygen time (seconds)............: " <<  time_B-time_A << std::endl;
    std::cout << "Total wall time (seconds)........: " <<  profiler.walltime() << std::endl;
    std::cout << "RAM usage (MB)...................: " <<  profiler.maxrss()  / 1e6 << std::endl;
}
