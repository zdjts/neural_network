// 作用：将一张图片（如bmp/png）转换为28x28的csv像素行，再从csv还原为图片，检查转换效果

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "../src/stb/LoadImage.h"
#include "../src/stb/SaveImage.h"
#include "../src/core/Matrix.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "用法: " << argv[0] << " 图片路径\n";
        return 1;
    }
    std::string image_path = argv[1];

    // 1. 加载图片并转换为28x28像素矩阵
    LoadImage loader(image_path);
    Matrix m = loader.get_Matrix(); // 1x784, float, [0,1]

    // 2. 保存为csv一行
    std::string csv_path = "image_to_csv_row.csv";
    std::ofstream fout(csv_path);
    for (int i = 0; i < 784; ++i) {
        fout << static_cast<int>(m(0, i) * 255);
        if (i != 783) fout << ",";
    }
    fout << std::endl;
    fout.close();
    std::cout << "已保存csv: " << csv_path << std::endl;

    // 3. 从csv读取像素并还原为图片
    std::ifstream fin(csv_path);
    std::string line;
    if (!std::getline(fin, line)) {
        std::cerr << "读取csv失败\n";
        return 1;
    }
    fin.close();

    std::vector<float> pixels;
    size_t pos = 0, last = 0;
    while ((pos = line.find(',', last)) != std::string::npos) {
        pixels.push_back(std::stof(line.substr(last, pos - last)) / 255.0f);
        last = pos + 1;
    }
    // 最后一个像素
    if (last < line.size())
        pixels.push_back(std::stof(line.substr(last)) / 255.0f);

    if (pixels.size() != 784) {
        std::cerr << "像素数量错误: " << pixels.size() << std::endl;
        return 1;
    }

    Matrix m2(1, 784);
    for (int i = 0; i < 784; ++i) m2(0, i) = pixels[i];

    std::string out_img = "restored_from_csv.png";
    SaveImage::save(out_img, m2, 28, 28);
    std::cout << "已保存还原图片: " << out_img << std::endl;

    return 0;
}