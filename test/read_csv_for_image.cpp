// ====================================
// 该脚本的作用是测试从CSV文件中读取指定行的数据，并将其转换为图像保存o
// ====================================
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "../src/core/Matrix.h"
#include "../src/stb/SaveImage.h"

// 简单读取一行csv为vector<float>
std::vector<float> read_csv_row(const std::string& line) {
    std::vector<float> result;
    std::stringstream ss(line);
    std::string cell;
    while (std::getline(ss, cell, ',')) {
        result.push_back(std::stof(cell));
    }
    return result;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "用法: " << argv[0] << " csv文件路径 行号(从0开始)\n";
        return 1;
    }
    std::string csv_path = argv[1];
    int row_idx = std::stoi(argv[2]);

    std::ifstream fin(csv_path);
    if (!fin) {
        std::cerr << "无法打开文件: " << csv_path << std::endl;
        return 1;
    }

    std::string line;
    int cur = 0;
    while (std::getline(fin, line)) {
        if (cur == row_idx) {
            auto values = read_csv_row(line);
            // 如果有标签，假设第一个是label，后面是像素
            int offset = (values.size() == 785) ? 1 : 0;
            int label = (offset == 1) ? static_cast<int>(values[0]) : -1; // 如果有标签，取第一个值
            Matrix m(1, 784);
            for (int i = 0; i < 784; ++i) {
                // 假设csv像素是0~255，归一化到0~1
                m(0, i) = values[i + offset] / 255.0f;
            }
            std::string out_path = "csv_row_" + std::to_string(row_idx) + "_" + std::to_string(label) + ".png";
            SaveImage::save(out_path, m, 28, 28);
            std::cout << "已保存图片: " << out_path << std::endl;
            return 0;
        }
        ++cur;
    }
    std::cerr << "未找到指定行: " << row_idx << std::endl;
    return 1;
}