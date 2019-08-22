#include "idash.h"

int main(int argc, char** argv) {
    Profiler profiler;
    std::string modelFile = MODEL_FILE;
    if (argc>=2) modelFile = argv[1];
    std::cout << "using model dir: " << modelFile << std::endl;

    IdashParams params;
    Model model;
    EncryptedData enc_data;
    EncryptedPredictions enc_preds;
    read_params(params, PARAMS_FILE);
    read_model(model, params, modelFile);
    read_encrypted_data(enc_data, params, ENCRYPTED_DATA_FILE);
    const double time_start_cloud = profiler.walltime();
    cloud_compute_score(enc_preds, enc_data, model, params);
    const double time_end_cloud = profiler.walltime();
    write_encrypted_predictions(enc_preds, params, ENCRYPTED_PREDICTION_FILE);
    const double time_write_end = profiler.walltime();

    std::cout << "----------------- BENCHMARK ----------------- " << std::endl;
    std::cout << "fhe wall time (seconds)..........: " <<  time_end_cloud-time_start_cloud << std::endl;
    std::cout << "serialization wall time (seconds): " <<  time_write_end - time_end_cloud + time_start_cloud << std::endl;
    std::cout << "total wall time (seconds)........: " <<  time_write_end << std::endl;
    std::cout << "RAM (MB).........................: " <<  profiler.maxrss() / 1e6 << std::endl;

}

