#include "main_parameters.h"
#include "houghgate.h"

#include <stdlib.h>

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

// YUV thresholds for default color filter
#ifndef HOUGHGATE_YMIN
#define HOUGHGATE_YMIN 84
#endif
#ifndef HOUGHGATE_YMAX
#define HOUGHGATE_YMAX 255
#endif
#ifndef HOUGHGATE_UMIN
#define HOUGHGATE_UMIN 79
#endif
#ifndef HOUGHGATE_UMAX
#define HOUGHGATE_UMAX 128
#endif
#ifndef HOUGHGATE_VMIN
#define HOUGHGATE_VMIN 56
#endif
#ifndef HOUGHGATE_VMAX
#define HOUGHGATE_VMAX 122
#endif


// Convenience macros for YUV422
#define PIXEL_U(img,x,y) ( ((uint8_t*)((img)->buf))[4*(int)((x)/2) + 2*(y)*(img)->w] )
#define PIXEL_V(img,x,y) ( ((uint8_t*)((img)->buf))[4*(int)((x)/2) + 2*(y)*(img)->w + 2] )
#define PIXEL_Y(img,x,y) ( ((uint8_t*)((img)->buf))[2*(x) + 1 + 2*(y)*(img)->w] )

// Internal types
struct pointu16_t {
  uint16_t x;
  uint16_t y;
};

/**
 * Test whether pixel (x,y) in image img belongs to the gate
 * @param img
 * @param x
 * @param y
 * @return nonzero (1) if inlier, 0 if outlier
 */
static uint8_t isgate_yuv(const struct image_t *img, uint16_t x, uint16_t y) {
  uint8_t score = 0;
  if(HOUGHGATE_YMIN <= PIXEL_Y(img, x, y) && PIXEL_Y(img, x, y) <= HOUGHGATE_YMAX &&
      HOUGHGATE_UMIN <= PIXEL_U(img, x, y) && PIXEL_U(img, x, y) <= HOUGHGATE_UMAX &&
      HOUGHGATE_VMIN <= PIXEL_V(img, x, y) && PIXEL_V(img, x, y) <= HOUGHGATE_VMAX) {
    score = 1;
  }
  return score;
}

static uint8_t sample_pixel(const struct image_t *img, struct pointu16_t *pt) {
  for(uint_fast16_t iter = HOUGHGATE_SAMPLE_ITER; iter > 0; --iter) {
    pt->x = rand() % img->w;
    pt->y = rand() % img->h;
    if(HOUGHGATE_INLIER(img, pt->x, pt->y)) {
      return 1;
    }
  }
  return 0;
}

struct houghresult_t houghgate(const struct image_t *img) {
  struct houghresult_t res;
  return res;
}
