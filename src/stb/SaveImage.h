#pragma once

#include <string>
#include "Matrix.h"

// 保存图片为灰度bmp/png
class SaveImage {
 public:
  // 将矩阵保存为图片，默认保存为28x28灰度图
  // file_path: 保存路径，支持.bmp/.png等
  // m: 输入矩阵，shape为(1, 784)或(28, 28)
  // width/height: 图片宽高，默认28
  static void save(const std::string& file_path,  Matrix& m, int width = 28, int height = 28);
};