#include "main_parameters.h"
#include "houghgate.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

// Number of attempts at random sampling 1 pixel
#ifndef HOUGHGATE_SAMPLE_ITER
#define HOUGHGATE_SAMPLE_ITER 100
#endif

// Downsample factor for accumulator size. Use 1, 2, 4 or 8
#ifndef HOUGHGATE_DOWNSAMPLE_FACTOR
#define HOUGHGATE_DOWNSAMPLE_FACTOR 4
#endif
#define ACCUMULATOR_WIDTH (IMAGE_WIDTH / HOUGHGATE_DOWNSAMPLE_FACTOR)
#define ACCUMULATOR_HEIGHT (IMAGE_HEIGHT / HOUGHGATE_DOWNSAMPLE_FACTOR)
#define ACCUMULATOR_AT(x, y) (accumulator[(x) + (y) * ACCUMULATOR_WIDTH])

// Max number of pixels to sample
#ifndef HOUGHGATE_MAX_SAMPLES
#define HOUGHGATE_MAX_SAMPLES 100
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

struct pointf_t {
  float x;
  float y;
};

#define PT_ADD(o, a, b) { (o).x = (a).x + (b).x; (o).y = (a).y + (b).y; }
#define PT_SUB(o, a, b) { (o).x = (a).x - (b).x; (o).y = (a).y - (b).y; }
#define PT_SMUL(o, a, s) { (o).x = (a).x * (s); (o).y = (a).y * (s); }
#define PT_SDIV(o, a, s) { (o).x = (a).x / (s); (o).y = (a).y / (s); }

// Accumulator and other buffers
static uint16_t accumulator[ACCUMULATOR_WIDTH * ACCUMULATOR_HEIGHT];
static struct pointu16_t points[HOUGHGATE_MAX_SAMPLES];
static uint_fast16_t points_count;


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

/**
 * Sample random pixel according to HOUGHGATE_INLIER()
 * @param img
 * @param pt: struct with pixel coordinates
 * @return RET_OK or RET_ERR
 */
static uint8_t sample_pixel(const struct image_t *img, struct pointu16_t *pt) {
  for(uint_fast16_t iter = HOUGHGATE_SAMPLE_ITER; iter > 0; --iter) {
    pt->x = rand() % img->w;
    pt->y = rand() % img->h;
    if(HOUGHGATE_INLIER(img, pt->x, pt->y)) {
      return RET_OK;
    }
  }
  return RET_ERR;
}

uint8_t houghgate(const struct image_t *img, struct houghresult_t *result) {
  // Initialize buffers
  memset(accumulator, sizeof(accumulator), 0);
  points_count = 0;
  result->inliers = 0;
  result->center.x = 0;
  result->center.y = 0;
  // Sample first pixel
  if(sample_pixel(img, &points[0])) return RET_ERR;
  ++points_count;
  do {
    // Sample next pixel
    if(points_count >= HOUGHGATE_MAX_SAMPLES || sample_pixel(img, &points[points_count])) break;
    ++points_count;
    // Add Hough transform with older pixels to accumulator
    for(uint_fast16_t i = 0; i < points_count - 1; ++i) {
      struct pointf_t midpt;
      midpt.x = ((float)points[points_count - 1].x + (float)points[i].x) / 2.0 / (float)HOUGHGATE_DOWNSAMPLE_FACTOR;
      midpt.y = ((float)points[points_count - 1].y + (float)points[i].y) / 2.0 / (float)HOUGHGATE_DOWNSAMPLE_FACTOR;
      struct pointf_t orth;
      orth.x = (float)points[points_count - 1].y - (float)points[i].y; // Note: y instead of x to find orthogonal!
      orth.y = -((float)points[points_count - 1].x - (float)points[i].x);
      if(abs(orth.x) > abs(orth.y)) { // Along x
        for(uint_fast16_t x = 0; x < ACCUMULATOR_WIDTH; ++x) {
          uint_fast16_t y = round(midpt.y + (x - midpt.x) / orth.x * orth.y);
          if(0 <= y && y < ACCUMULATOR_HEIGHT) {
            if(ACCUMULATOR_AT(x, y) < UINT16_MAX) ++ACCUMULATOR_AT(x, y);
            if(ACCUMULATOR_AT(x, y) > result->inliers) {
              result->inliers = ACCUMULATOR_AT(x, y);
              result->center.x = x * HOUGHGATE_DOWNSAMPLE_FACTOR;
              result->center.y = y * HOUGHGATE_DOWNSAMPLE_FACTOR;
            }
          }
        }
      } else { // Along y
        for(uint_fast16_t y = 0; y < ACCUMULATOR_HEIGHT; ++y) {
          uint_fast16_t x = round(midpt.x + (y - midpt.y) / orth.y * orth.x);
          if(0 <= x && x < ACCUMULATOR_WIDTH) {
            if(ACCUMULATOR_AT(x, y) < UINT16_MAX) ++ACCUMULATOR_AT(x, y);
            if(ACCUMULATOR_AT(x, y) > result->inliers) {
              result->inliers = ACCUMULATOR_AT(x, y);
              result->center.x = x * HOUGHGATE_DOWNSAMPLE_FACTOR;
              result->center.y = y * HOUGHGATE_DOWNSAMPLE_FACTOR;
            }
          }
        }
      }
    }
  } while (1); // TODO check clock
  return RET_OK;
}


// DEBUG FUNCTIONS, manually add external declaration to calling code
void houghgate_debug_get_accumulator(uint16_t **acc, int *w, int *h) {
  *acc = accumulator;
  *w = ACCUMULATOR_WIDTH;
  *h = ACCUMULATOR_HEIGHT;
}
