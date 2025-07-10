#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "LoadImage.h"

#include <stdexcept>
#include <utility>
#include <vector>

#include "Matrix.h"

LoadImage::LoadImage(const std::string& image_path)
    : data_(image_to_mnist(image_path)) {}
Matrix LoadImage::image_to_mnist(const std::string& image_path) {
  int width, height, channels;
  unsigned char* raw_pixels =
      stbi_load(image_path.c_str(), &width, &height, &channels, 0);
  if (raw_pixels == nullptr) {
    throw std::runtime_error(image_path + "图片无法加载，运行失败\n");
  }

  std::vector<unsigned char> v(width * height);

  if (channels > 1) {
    for (int i = 0; i < width * height; ++i) {
      unsigned char r = raw_pixels[i * channels];
      unsigned char g = raw_pixels[i * channels + 1];
      unsigned char b = raw_pixels[i * channels + 2];
      // 应用标准灰度转换公式: Gray = 0.299*R + 0.587*G + 0.114*B
      v[i] = static_cast<unsigned char>(0.299 * r + 0.587 * g + 0.114 * b);
    }
  } else {
    v.assign(raw_pixels, raw_pixels + width * height);
  }
  stbi_image_free(raw_pixels);
  const int target_size = 28;
  std::vector<unsigned char> resized_pixels(target_size * target_size);

  // 调用 stb_image_resize.h 提供的缩放函数
  stbir_resize_uint8_linear(v.data(), width, height, 0, resized_pixels.data(),
                            target_size, target_size, 0, STBIR_1CHANNEL);

  // 4. 颜色反转与归一化。
  std::vector<float> final_matrix(target_size * target_size);
  for (int i = 0; i < target_size * target_size; ++i) {
    // MNIST 标准是白字黑底，我们的图片通常是黑字白底，因此需要反转 (255 -
    // pixel)。 同时归一化到 [0.0, 1.0] 的浮点数范围。
    /*final_matrix[i] = resized_pixels[i] / 255.0;*/
    final_matrix[i] = (255 - resized_pixels[i]) / 255.0;
  }
  Matrix m(1, target_size * target_size);

  m.get_data().assign(final_matrix.begin(), final_matrix.end());
  return m;
}

Matrix& LoadImage::get_Matrix() { return data_; }
