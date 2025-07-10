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
  Matrix image_to_mnist(const std::string& image_path);
  Matrix& get_Matrix();
};
#endif  // !LOADIMAGE_H
