#include <unordered_map>
#include <string>
#include <vector>

/**
 * @brief      Read a vowpal wabbit model file in human-readable form
 *
 * @param      coefs      The coefficients, where coefs[k] is a (feature name,
 *                        value) map for predicting output==k (k=0..2)
 * @param[in]  file_name  The file name
 */
void read(std::vector<std::unordered_map<std::string, float>>& coefs, const std::string& file_name);
