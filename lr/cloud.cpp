#include "idash.h"



int main() {
    IdashParams params;
    Model model;
    EncryptedData enc_data;
    EncryptedPredictions enc_preds;
    read_params(params, "params_file");
    read_model(model, params, "../ml/model/hr/10k");
    read_encrypted_data(enc_data, params, "enc_data_file");
    cloud_compute_score(enc_preds, enc_data, model, params);
    write_encrypted_predictions(enc_preds, params, "enc_preds_filename");
}

