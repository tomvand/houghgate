#include <opencv2/opencv.hpp>

#include <iostream>

extern "C" {
#include "houghgate.h"
#include "image.h"
#include "main_parameters.h"

struct pointu16_t {
  uint16_t x;
  uint16_t y;
};
extern void houghgate_debug_get_accumulator(uint16_t **acc, int *w, int *h);
extern void houghgate_debug_get_points(struct pointu16_t **pts, int *pts_cnt);
}

void mat2yuv422(cv::Mat* image, uint8_t* image_buffer)
{
  cv::Mat yuv;
  cv::cvtColor(*image, yuv, cv::COLOR_BGR2YUV);
//  image->copyTo(yuv);
  // Put image values in array, just like in the stereoboard
  int x, y, idx = 0;

  for (y = 0; y < yuv.rows; y++) {
    for (x = 0; x < yuv.cols; x++)
    {
      if(idx % 4 == 0){
        image_buffer[idx] = (uint8_t)yuv.at<cv::Vec3b>(y, x)[1];
      } else {
        image_buffer[idx] = (uint8_t)yuv.at<cv::Vec3b>(y, x)[2];
      }
      image_buffer[idx+1] = (uint8_t)yuv.at<cv::Vec3b>(y, x)[0];
      idx += 2;
    }
  }
}

int main(int argc, char **argv) {
  // Open test images
  cv::VideoCapture cap("/home/tom/Dropbox/PhD/IMAV/data/gates/color6%02d.png");
  if (!cap.isOpened()) {
    printf("Couldn't open images\n");
    return -1;
  }

  // Loop through images
  cv::Mat img;
  uint8_t buf[IMAGE_WIDTH * IMAGE_HEIGHT * 2] = {0};
  struct image_t img_s;
  img_s.buf = buf;
  img_s.w = IMAGE_WIDTH;
  img_s.h = IMAGE_HEIGHT;
  img_s.buf_size = IMAGE_WIDTH*IMAGE_HEIGHT*2;
  img_s.type = IMAGE_YUV422;
  for(;;) {
    cap >> img;
    if(img.empty()) break;
    cv::resize(img, img, cv::Size(), (float)IMAGE_WIDTH/img.cols, (float)IMAGE_HEIGHT/img.rows);
    mat2yuv422(&img, buf);

    struct houghresult_t res;
    if(houghgate(&img_s, &res)) {
      std::cout << "RET_ERR" << std::endl;
    } else {
      cv::circle(img, cv::Point(res.center.x, res.center.y), 3, cv::Scalar(255, 255, 255));
    }

    std::cout << "X = " << res.center.x << ", Y = " << res.center.y << std::endl;
    std::cout << "samples = " << res.samples << ", inliers = " << res.inliers << std::endl;
    std::cout << "===================" << std::endl;

    cv::namedWindow("image", cv::WINDOW_NORMAL);
    cv::imshow("image", img);

    cv::namedWindow("points", cv::WINDOW_NORMAL);
    cv::Mat points;
    img.copyTo(points);
    struct pointu16_t *pts;
    int pts_cnt;
    houghgate_debug_get_points(&pts, &pts_cnt);
    for(int i = 0; i < pts_cnt; ++i) {
      points.at<cv::Vec3b>(cv::Point(pts[i].x, pts[i].y)) = cv::Vec3b(255, 255, 255);
    }
    cv::imshow("points", points);

    cv::namedWindow("accumulator", cv::WINDOW_NORMAL);
    uint16_t *accumulator;
    int acc_w, acc_h;
    houghgate_debug_get_accumulator(&accumulator, &acc_w, &acc_h);
    cv::Mat_<uint16_t> acc(acc_h, acc_w, CV_16UC1);
    for(int x = 0; x < acc_w; ++x) {
      for(int y = 0; y < acc_h; ++y) {
        acc.at<uint16_t>(y, x) = accumulator[x + y * acc_w];
      }
    }
    cv::Mat acc_color;
    cv::normalize(acc, acc, 0, UINT16_MAX, cv::NORM_MINMAX);
    cv::applyColorMap(acc, acc_color, cv::COLORMAP_PARULA);
    cv::imshow("accumulator", acc_color);

    cv::waitKey(0);
  }
}
