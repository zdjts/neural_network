/**
 * @file    test/test_transform_image_csv.cpp
 * @author  曾瑞安
 * @brief   验证完整的图像预处理链路：从图像到CSV，再从CSV还原回图像。
 *
 * @details
 * 图像预处理验证:
 * 此脚本用于测试完整的图像预处理链路。它将一张本地图片（例如，自己
 * 用画图工具创建的数字图片）转换为CSV格式，然后再从该CSV文件中读取
 * 数据并还原为图片。
 *
 * 通过对比原始图片和还原后的图片，可以验证 LoadImage 类中的灰度化、
 * 尺寸缩放、颜色反转和归一化等步骤是否都按预期工作。
 *
 * 验证流程：
 * 1.  加载一张本地图片。
 * 2.  使用 `LoadImage` 类进行处理，得到一个归一化的1x784矩阵。
 * 3.  将矩阵数据写入一个单行的CSV文件。
 * 4.  从该CSV文件中读取数据。
 * 5.  将读取的数据还原为1x784矩阵。
 * 6.  将此矩阵保存为一张新的PNG图片。
 * 7.  用户通过肉眼对比原始图和生成图，验证流程的正确性。
 */
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../src/core/Matrix.h"
#include "../src/stb/LoadImage.h"
#include "../src/stb/SaveImage.h"

// 从CSV行字符串中解析像素数据
std::vector<float> read_pixels_from_csv_line(const std::string& line) {
  std::vector<float> pixels;
  std::stringstream ss(line);
  std::string cell;
  while (std::getline(ss, cell, ',')) {
    try {
      // 从CSV读取时，将[0, 255]范围的整数像素值转换回[0, 1]的浮点数
      pixels.push_back(std::stof(cell) / 255.0f);
    } catch (const std::exception& e) {
      std::cerr << "警告: 无法将 '" << cell
                << "' 转换为数字。错误: " << e.what() << std::endl;
    }
  }
  return pixels;
}

int main(int argc, char** argv) {
  // --- 1. 参数校验 ---
  if (argc < 2) {
    std::cerr << "错误: 参数不足。\n"
              << "用法: " << argv[0] << " <image_path>\n"
              << "示例: " << argv[0] << " my_digit.png\n";
    return 1;
  }
  std::string image_path = argv[1];

  try {
    // --- 2. 加载图片并转换为矩阵 ---
    std::cout << "正在加载图片: " << image_path << " 并进行预处理..."
              << std::endl;
    LoadImage loader(image_path);
    Matrix image_matrix = loader.get_Matrix();  // 尺寸 1x784, 值范围 [0, 1]

    // --- 3. 将矩阵数据写入CSV文件 ---
    std::string csv_path = "temp_image_to_csv.csv";
    std::ofstream fout(csv_path);
    if (!fout.is_open()) {
      std::cerr << "错误: 无法创建并写入CSV文件: " << csv_path << std::endl;
      return 1;
    }

    for (int i = 0; i < image_matrix.get_cols(); ++i) {
      // 将[0, 1]的浮点数转换回[0, 255]的整数范围进行存储
      fout << static_cast<int>(image_matrix(0, i) * 255);
      if (i < image_matrix.get_cols() - 1) {
        fout << ",";
      }
    }
    fout << std::endl;
    fout.close();
    std::cout << "图像已成功转换为CSV格式并保存至: " << csv_path << std::endl;

    // --- 4. 从CSV文件读回数据 ---
    std::ifstream fin(csv_path);
    if (!fin.is_open()) {
      std::cerr << "错误: 无法打开并读取CSV文件: " << csv_path << std::endl;
      return 1;
    }
    std::string line;
    if (!std::getline(fin, line)) {
      std::cerr << "错误: 读取CSV文件失败或文件为空。\n";
      fin.close();
      return 1;
    }
    fin.close();

    std::vector<float> pixels = read_pixels_from_csv_line(line);
    if (pixels.size() != 784) {
      std::cerr << "错误: 从CSV读取的像素数量不正确: " << pixels.size()
                << " (应为784)" << std::endl;
      return 1;
    }

    Matrix restored_matrix(1, 784);
    for (size_t i = 0; i < pixels.size(); ++i) {
      restored_matrix(0, i) = pixels[i];
    }

    // --- 5. 保存还原后的图片以供对比 ---
    std::string output_image_path = "restored_from_csv.png";
    SaveImage::save(output_image_path, restored_matrix, 28, 28);
    std::cout << "已从CSV数据中还原图片并保存至: " << output_image_path
              << std::endl;
    std::cout << "\n验证步骤: 请用图像查看器对比原始图片 '" << image_path
              << "' 和还原后的图片 '" << output_image_path << "'。"
              << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "处理过程中发生严重错误: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
