#include "noise.h"  // 包含对应的头文件

#include <algorithm>  // 用于 std::min, std::max
#include <cmath>   // 用于 std::sqrt, std::abs, std::round, std::cos, std::sin
#include <random>  // 用于 std::random_device, std::mt19937, std::uniform_int_distribution, std::uniform_real_distribution, std::normal_distribution

// 定义 PI 常量，确保所有地方都使用相同的 PI 值
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// 随机平移（上下左右填0）
void random_shift(Matrix& m, int max_shift) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  // 假设 image 是一个方形矩阵，所以直接用 get_rows() 或 get_cols() 都可以
  int size = m.get_rows();
  std::uniform_int_distribution<int> shift_dist(-max_shift, max_shift);
  int dx = shift_dist(gen);
  int dy = shift_dist(gen);

  if (dx == 0 && dy == 0) return;  // 无需平移

  Matrix shifted_image(size, size, 0.0f);  // 创建一个全零的新矩阵
  for (int y = 0; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      int src_y = y - dy;
      int src_x = x - dx;
      // 确保源坐标在边界内
      if (src_y >= 0 && src_y < size && src_x >= 0 && src_x < size) {
        shifted_image(y, x) = m(src_y, src_x);
      }
    }
  }
  m = shifted_image;  // 用处理后的图像替换原始图像
}

// 随机缩放（中心缩放，空白区域填0）
void random_scale(Matrix& m, float min_scale, float max_scale) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  int size = m.get_rows();
  std::uniform_real_distribution<float> scale_dist(min_scale, max_scale);
  float scale = scale_dist(gen);

  if (std::abs(scale - 1.0f) < 1e-6) return;

  int new_size = static_cast<int>(size * scale);
  if (new_size < 1) new_size = 1;

  Matrix scaled_image(new_size, new_size, 0.0f);  // 初始化为0
  // 使用最近邻插值进行缩放
  for (int y = 0; y < new_size; ++y) {
    for (int x = 0; x < new_size; ++x) {
      int src_y = static_cast<int>(y / scale);
      int src_x = static_cast<int>(x / scale);
      scaled_image(y, x) =
          m(std::min(src_y, size - 1), std::min(src_x, size - 1));
    }
  }

  // 将缩放后的图像粘贴到原始大小的画布中央
  // 注意：这里 Matrix 的 clear() 方法需要确保能正确清空矩阵数据并重置尺寸
  // 如果 Matrix::clear() 只是清空数据但不重置尺寸，或者不能保证原有 m 的尺寸，
  // 可能会有问题。更安全的方式是直接创建新的 Matrix 对象。
  // 但是，由于 m = scaled_image;
  // 这样的赋值操作会处理尺寸和数据，所以此处逻辑可能没有问题。 如果 m
  // 赋值后尺寸不变，且 clear() 行为符合预期，这段逻辑是正确的。 假设 m.clear()
  // 会把 m 的所有元素设为 0.0f
  m.clear();  // 先清空原始图像

  int offset_y = (size - new_size) / 2;
  int offset_x = (size - new_size) / 2;

  for (int y = 0; y < new_size; ++y) {
    for (int x = 0; x < new_size; ++x) {
      int dest_y = y + offset_y;
      int dest_x = x + offset_x;
      if (dest_y >= 0 && dest_y < size && dest_x >= 0 && dest_x < size) {
        m(dest_y, dest_x) = scaled_image(y, x);
      }
    }
  }
}

// 添加高斯噪声
void add_gaussian_noise(Matrix& m, float mean, float stddev) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::normal_distribution<float> noise_dist(mean, stddev);
  for (int y = 0; y < m.get_rows(); ++y) {
    for (int x = 0; x < m.get_cols(); ++x) {
      float noise = noise_dist(gen);
      float new_value = m(y, x) + noise;
      // 将结果裁剪到 [0, 1] 范围内
      m(y, x) = std::max(0.0f, std::min(1.0f, new_value));
    }
  }
}

// 随机旋转（空白区域填0）
void random_rotate(Matrix& m, float max_angle) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<float> angle_dist(-max_angle, max_angle);
  float angle_degrees = angle_dist(gen);
  if (std::abs(angle_degrees) < 1e-6) return;

  float angle_rad = angle_degrees * M_PI / 180.0f;
  float cos_a = std::cos(angle_rad);
  float sin_a = std::sin(angle_rad);

  int size = m.get_rows();
  float center = (size - 1) / 2.0f;  // 确保中心点是浮点数，方便计算

  Matrix rotated_image(size, size, 0.0f);  // 初始化为0
  for (int y = 0; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      // 将目标坐标转换为以中心为原点的坐标
      float x_c = x - center;
      float y_c = y - center;

      // 应用反向旋转变换，找到源坐标
      float src_x_c = x_c * cos_a + y_c * sin_a;
      float src_y_c = -x_c * sin_a + y_c * cos_a;

      // 将源坐标转换回矩阵索引
      float src_x = src_x_c + center;
      float src_y = src_y_c + center;

      // 使用最近邻插值
      int isrc_x = static_cast<int>(std::round(src_x));
      int isrc_y = static_cast<int>(std::round(src_y));

      if (isrc_x >= 0 && isrc_x < size && isrc_y >= 0 && isrc_y < size) {
        rotated_image(y, x) = m(isrc_y, isrc_x);
      }
    }
  }
  m = rotated_image;
}

// 随机反转颜色（黑白翻转） - 这个函数你没有提供实现，我用你之前的代码补全
void random_invert(Matrix& m, float prob) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<float> p_dist(0.0f, 1.0f);  // 避免与参数p冲突
  if (p_dist(gen) < prob) {
    for (int i = 0; i < m.get_rows() * m.get_cols(); ++i) {  // 遍历所有元素
      m.get_data()[i] =
          1.0f - m.get_data()[i];  // 假设 get_data() 返回 std::vector<float>&
    }
  }
}

// 随机擦除（随机区域置0）
void random_erasing(Matrix& m, float erase_prob, float min_area_ratio,
                    float max_area_ratio) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<float> p(0.0f, 1.0f);
  if (p(gen) > erase_prob) return;

  int size = m.get_rows();
  float total_area = static_cast<float>(size * size);
  std::uniform_real_distribution<float> area_dist(min_area_ratio,
                                                  max_area_ratio);
  float target_erase_area = area_dist(gen) * total_area;

  // 随机确定擦除矩形的宽高比
  std::uniform_real_distribution<float> aspect_ratio_dist(0.3f, 1.0f / 0.3f);
  float aspect_ratio = aspect_ratio_dist(gen);

  int erase_h = static_cast<int>(std::sqrt(target_erase_area * aspect_ratio));
  int erase_w = static_cast<int>(std::sqrt(target_erase_area / aspect_ratio));
  if (erase_w < 1 || erase_h < 1) return;  // 确保宽高有效

  std::uniform_int_distribution<int> x_dist(
      0, std::max(0, size - erase_w));  // 确保范围有效
  std::uniform_int_distribution<int> y_dist(
      0, std::max(0, size - erase_h));  // 确保范围有效
  int x0 = x_dist(gen);
  int y0 = y_dist(gen);

  for (int y = y0; y < y0 + erase_h; ++y) {
    for (int x = x0; x < x0 + erase_w; ++x) {
      // 确保擦除区域在图像边界内
      if (y >= 0 && y < size && x >= 0 && x < size) {
        m(y, x) = 0.0f;  // 擦除为黑色
      }
    }
  }
}
