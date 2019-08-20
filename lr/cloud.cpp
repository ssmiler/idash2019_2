#include "idash.h"

int main(int argc, char** argv) {
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
    cloud_compute_score(enc_preds, enc_data, model, params);
    write_encrypted_predictions(enc_preds, params, ENCRYPTED_PREDICTION_FILE);
}

