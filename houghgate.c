#include "main_parameters.h"
#include "houghgate.h"

// Number of attempts at random sampling 1 pixel
#ifndef HOUGHGATE_SAMPLE_ITER
#define HOUGHGATE_SAMPLE_ITER 100
#endif

// Downsample factor for accumulator size. Use 1, 2, 4 or 8
#ifndef HOUGHGATE_DOWNSAMPLE_FACTOR
#define HOUGHGATE_DOWNSAMPLE_FACTOR 4
#endif

// Function to check pixel as inlier
#ifndef HOUGHGATE_INLIER
#define HOUGHGATE_INLIER(img_ptr, x, y) (isgate_yuv(img_ptr, x, y))
#endif


struct houghresult_t houghgate(const struct image_t *img) {
  struct houghresult_t res;
  return res;
}
