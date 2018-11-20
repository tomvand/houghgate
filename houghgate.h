#ifndef HOUGHGATE_H
#define HOUGHGATE_H

#include "image.h"

#include <stdint.h>

#define RET_OK 0
#define RET_ERR 1

struct houghresult_t {
  uint16_t inliers;
  struct point_t center;
  uint8_t radius;
};

uint8_t houghgate(const struct image_t *img, struct houghresult_t *result);

#endif
