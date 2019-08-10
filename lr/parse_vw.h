#include <unordered_map>
#include <string>
#include <cstdint>

/**
 * @brief      Read a model file
 *
 * @param[in]  file_name  File name
 *
 * @return     Model coefficients in form (pos_snp, value) map for predicting
 *             output==k (k=0..2)
 */
std::unordered_map<std::string, int32_t> read(const std::string &file_name);

