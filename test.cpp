#include <opencv2/opencv.hpp>

#include <iostream>

extern "C" {
#include "houghgate.h"
#include "image.h"
#include "main_parameters.h"
extern void houghgate_debug_get_accumulator(uint16_t **acc, int *w, int *h);
}

void mat2yuv422(cv::Mat* image, uint8_t* image_buffer)
{
  cv::Mat yuv;
  cv::cvtColor(*image, yuv, cv::COLOR_BGR2YUV);
  // Put image values in array, just like in the stereoboard
  int x, y, idx = 0;

  for (y = 0; y < image->rows; y++) {
    for (x = 0; x < image->cols; x++)
    {
      if(idx % 4 == 0){
        image_buffer[idx] = (uint8_t)image->at<cv::Vec3b>(y, x)[1];
      } else {
        image_buffer[idx] = (uint8_t)image->at<cv::Vec3b>(y, x)[2];
      }
      image_buffer[idx+1] = (uint8_t)image->at<cv::Vec3b>(y, x)[0];
      idx += 2;
    }
  }
}

int main(int argc, char **argv) {
  // Open test images
  cv::VideoCapture cap("/home/tom/Dropbox/PhD/IMAV/2018/data/gates1/color615.png");
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
    }

    std::cout << "X = " << res.center.x << ", Y = " << res.center.y << std::endl;
    std::cout << "inliers = " << res.inliers << std::endl;
    std::cout << "===================" << std::endl;

    cv::circle(img, cv::Point(res.center.x, res.center.y), 3, cv::Scalar(255, 255, 255));

    cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
    cv::imshow("image", img);

    cv::namedWindow("accumulator", cv::WINDOW_AUTOSIZE);
    uint16_t *accumulator;
    int acc_w, acc_h;
    houghgate_debug_get_accumulator(&accumulator, &acc_w, &acc_h);
    cv::Mat acc(acc_h, acc_w, CV_16U, accumulator);
    cv::Mat acc_color;
    cv::normalize(acc, acc, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    cv::applyColorMap(acc, acc_color, cv::COLORMAP_PARULA);
    cv::imshow("accumulator", acc_color);

    cv::waitKey(0);
  }
}
