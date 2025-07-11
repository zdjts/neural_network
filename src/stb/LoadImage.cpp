#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "LoadImage.h"

#include <stdexcept>
#include <utility>
#include <vector>

#include "Matrix.h"

LoadImage::LoadImage(const std::string& image_path, int num)
    : data_(image_to_mnist(image_path, num)) {}

Matrix LoadImage::image_to_mnist(const std::string& image_path, int num) {
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

  // 调用 stb_image_resize.h 提供的缩放函数，将图像缩放到 28x28
  stbir_resize_uint8_linear(v.data(), width, height, 0, resized_pixels.data(),
                            target_size, target_size, 0, STBIR_1CHANNEL);

  // ======== 在这里添加你的 3x3 卷积核预处理步骤 ========

  // 定义 3x3 模糊卷积核
  float kernel[3][3] = {{1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f},
                        {1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f},
                        {1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f}};

  // 创建一个临时缓冲区来存储卷积结果，保持 28x28 尺寸
  std::vector<unsigned char> convolved_pixels(target_size * target_size);

  // 执行 3x3 卷积 (这里使用零填充以保持输出尺寸不变)
  for (int i = 0; i < target_size; ++i) {    // 输出图像的行
    for (int j = 0; j < target_size; ++j) {  // 输出图像的列
      float sum_val = 0.0f;
      for (int k_row = -1; k_row <= 1; ++k_row) {    // 卷积核的行偏移
        for (int k_col = -1; k_col <= 1; ++k_col) {  // 卷积核的列偏移
          int pixel_row = i + k_row;                 // 原始图像中对应像素的行
          int pixel_col = j + k_col;                 // 原始图像中对应像素的列

          // 检查是否超出边界，如果超出则视为 0 (零填充)
          if (pixel_row >= 0 && pixel_row < target_size && pixel_col >= 0 &&
              pixel_col < target_size) {
            sum_val += resized_pixels[pixel_row * target_size + pixel_col] *
                       kernel[k_row + 1]
                             [k_col + 1];  // k_row+1, k_col+1 调整为核的索引
          }
        }
      }
      // 将卷积结果存入缓冲区，并确保在 0-255 范围内
      convolved_pixels[i * target_size + j] =
          static_cast<unsigned char>(std::max(0.0f, std::min(255.0f, sum_val)));
    }
  }

  // ======== 卷积预处理步骤结束 ========

  // 颜色反转与归一化。
  std::vector<float> final_matrix_data(target_size * target_size);
  for (int i = 0; i < target_size * target_size; ++i) {
    // MNIST 标准是白字黑底，我们的图片通常是黑字白底，因此需要反转 (255 -
    // pixel)。 同时归一化到 [0.0, 1.0] 的浮点数范围。
    final_matrix_data[i] =
        (255.0f - convolved_pixels[i]) / 255.0f;  // 使用卷积后的像素数据
  }

  Matrix m(1, target_size * target_size);
  m.get_data().assign(final_matrix_data.begin(), final_matrix_data.end());
  return m;
}

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

  // 根据图片亮度较暗进行拉高曝光度
  for (float& v : final_matrix) {
    v = std::pow(v, 0.1f);  // 让暗的更暗，亮的更亮（可调节指数）
  }

  Matrix m(1, target_size * target_size);

  m.get_data().assign(final_matrix.begin(), final_matrix.end());
  return m;
}

Matrix& LoadImage::get_Matrix() { return data_; }
