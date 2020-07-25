#ifndef KEYS_H
#define KEYS_H

#include <stddef.h>

typedef struct procy_key_info_t {
  const char* name;
  int value;
} procy_key_info_t;

void procy_get_keys(procy_key_info_t** buffer, size_t* len);

#endif
