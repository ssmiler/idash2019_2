#include "parse_vw.h"

#include <cassert>
#include <cstring>

using namespace std;

void read(vector<unordered_map<string, float>> &coefs,
          const string &file_name) {
  FILE *file = fopen(file_name.c_str(), "r");

  if (file == nullptr) {
    fprintf(stderr, "Cannot open file '%s'\n", file_name.c_str());
    exit(-1);
  }

  char buf[256];

  while (fgets(buf, 256, file)) {
    if (buf[0] == ':' and buf[1] == '0')
      break;
  }

  coefs = vector<unordered_map<string, float>>(3);

  while (fgets(buf, 256, file)) {
    char *ptr = strchr(buf, ':');
    assert(ptr);

    *ptr = '\0';
    ptr++;

    int a;
    float coef;

    sscanf(ptr, "%d:%f", &a, &coef);

    ptr = strchr(buf, '[');

    int target = 0;
    if (ptr != nullptr) {
      *ptr = '\0';
      sscanf(ptr+1, "%d", &target);
    }

    coefs[target][buf] = coef;
  }

  fclose(file);
}
