#ifndef HOUGHGATE_H
#define HOUGHGATE_H

#include "image.h"

#include <stdint.h>

struct houghresult_t {
  struct point_t center;
  uint8_t radius;
};

struct houghresult_t houghgate(const struct image_t *img);

#endif
