#include <cstdio>
#include "parse_vw.h"

using namespace std;

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Usage: %s <*.model.hr>\n", argv[0]);
    exit(0);
  }

  vector<unordered_map<string, float>> coeffs;
  read(coeffs, argv[1]);

  printf("Number of targets in model '%s': %ld\n", argv[1], coeffs.size());
  for (int target = 0; target < coeffs.size(); ++target) {
    printf("Target %d - %ld coefficients:\n", target, coeffs[target].size());
    float coef_min = 1e20, coef_max = 1e-20;
    for (auto& p: coeffs[target]) {
      printf("\t%s %f\n", p.first.c_str(), p.second);
      coef_min = min(coef_min, p.second);
      coef_max = max(coef_max, p.second);
    }
    printf("\tmin-max: %f %f\n", coef_min, coef_max);
  }
}
