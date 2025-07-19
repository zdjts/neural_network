/**
 * @file    test/read_csv_for_image.cpp
 * @brief   从CSV文件（如mnist_train.csv）中读取并可视化单行图像数据。
 *
 * @details
 * 我们利用这个脚本，从一个包含图像像素数据的CSV文件中随机抽取一行，
 * 并将其像素值渲染成一张图片。通过肉眼观察生成的图片，可以快速判断
 * 数据加载和图像解析流程是否正确。
 *
 * 验证流程：
 * 1.  接收CSV文件路径和行号作为命令行参数。
 * 2.  打开并读取指定的CSV文件。
 * 3.  定位到用户指定的行。
 * 4.  解析该行，提取像素值（通常第一个值为标签，其余为像素）。
 * 5.  将像素值转换为一个28x28的矩阵，并进行归一化处理。
 * 6.  将该矩阵保存为PNG图像文件，文件名中包含行号和标签。
 */
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../src/core/Matrix.h"
#include "../src/stb/SaveImage.h"

std::vector<float> read_csv_row(const std::string& line) {
  std::vector<float> result;
  std::stringstream ss(line);
  std::string cell;
  while (std::getline(ss, cell, ',')) {
    try {
      result.push_back(std::stof(cell));
    } catch (const std::invalid_argument& ia) {
      std::cerr << "警告: 无法将值 '" << cell << "' 转换为浮点数。"
                << std::endl;
    }
  }
  return result;
}

int main(int argc, char** argv) {
  // --- 1. 参数校验 ---
  if (argc < 3) {
    std::cerr << "错误: 参数不足。\n"
              << "用法: " << argv[0] << " <csv_file_path> <row_index>\n"
              << "示例: " << argv[0] << " ../data/mnist_train.csv 100\n";
    return 1;
  }

  std::string csv_path = argv[1];
  int row_to_extract = 0;
  try {
    row_to_extract = std::stoi(argv[2]);
    if (row_to_extract < 0) {
      std::cerr << "错误: 行号不能为负数。\n";
      return 1;
    }
  } catch (const std::exception& e) {
    std::cerr << "错误: 无效的行号 '" << argv[2] << "'。" << std::endl;
    return 1;
  }

  // --- 2. 打开并读取文件 ---
  std::ifstream fin(csv_path);
  if (!fin.is_open()) {
    std::cerr << "错误: 无法打开文件: " << csv_path << std::endl;
    return 1;
  }

  std::string line;
  int current_line_num = 0;
  while (std::getline(fin, line)) {
    if (current_line_num == row_to_extract) {
      auto values = read_csv_row(line);

      // MNIST数据集通常第一列是标签，后面784列是像素
      if (values.size() != 785) {
        std::cerr << "错误: 第 " << row_to_extract << " 行的数据列数 ("
                  << values.size() << ") 不符合预期的785列。" << std::endl;
        fin.close();
        return 1;
      }

      // --- 3. 数据解析与图像转换 ---
      int label = static_cast<int>(values[0]);
      Matrix image_matrix(1, 784);
      for (int i = 0; i < 784; ++i) {
        // 将像素值从 [0, 255] 归一化到 [0, 1]
        image_matrix(0, i) = values[i + 1] / 255.0f;
      }

      // --- 4. 保存图像 ---
      std::string output_filename = "image_from_row_" +
                                    std::to_string(row_to_extract) + "_label_" +
                                    std::to_string(label) + ".png";
      try {
        SaveImage::save(output_filename, image_matrix, 28, 28);
        std::cout << "图片已成功保存至: " << output_filename << std::endl;
      } catch (const std::exception& e) {
        std::cerr << "错误: 保存图片时发生异常: " << e.what() << std::endl;
        fin.close();
        return 1;
      }

      fin.close();
      return 0;  // 成功找到并处理完指定行
    }
    current_line_num++;
  }

  // 如果循环结束仍未找到指定行
  std::cerr << "错误: 未在文件中找到行号 " << row_to_extract
            << " (文件总行数: " << current_line_num << ")。" << std::endl;
  fin.close();
  return 1;
}
