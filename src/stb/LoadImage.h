#ifndef LOADIMAGE_H
#define LOADIMAGE_H
#include <string>

#include "../core/Matrix.h"
#include "./stb_image.h"
#include "./stb_image_resize2.h"

class LoadImage {
 private:
  Matrix data_;

 public:
  LoadImage(const std::string& image_path);
  LoadImage(const std::string& image_path, int num);
  Matrix image_to_mnist(const std::string& image_path);
  //
  // 使用一个卷积核后读取
  Matrix image_to_mnist(const std::string& image_path, int num);
  Matrix& get_Matrix();
};
#endif  // !LOADIMAGE_H
