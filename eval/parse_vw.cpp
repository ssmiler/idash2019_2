#include "parse_vw.h"

#include <cassert>
#include <cstring>

using namespace std;

unordered_map<string, int32_t> read(const string &file_name) {
  FILE *file = fopen(file_name.c_str(), "r");

  if (file == nullptr) {
    fprintf(stderr, "Cannot open file '%s'\n", file_name.c_str());
      abort();
  }

  unordered_map<string, int32_t> coefs;

  char buf[256];
  while (fgets(buf, 256, file)) {
    char target[256];
    int32_t value;

    sscanf(buf, "%s %d", target, &value);
    coefs[target] = value;
  }

  fclose(file);

  return coefs;
}
