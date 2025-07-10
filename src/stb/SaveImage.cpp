#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "SaveImage.h"
#include "stb_image_write.h"

#include <vector>
#include <stdexcept>

void SaveImage::save(const std::string& file_path,  Matrix& m, int width, int height) {
    if (m.get_rows() * m.get_cols() != width * height) {
        throw std::runtime_error("Matrix size does not match image size");
    }
    // 假设矩阵数据是float，范围[0,1]，转为uint8
    std::vector<unsigned char> pixels(width * height);
    const auto& data = m.get_data();
    for (int i = 0; i < width * height; ++i) {
        float v = data[i];
        if (v < 0) v = 0;
        if (v > 1) v = 1;
        pixels[i] = static_cast<unsigned char>(v * 255);
    }
    // 保存为png或bmp
    if (file_path.size() >= 4 && file_path.substr(file_path.size()-4) == ".png") {
        stbi_write_png(file_path.c_str(), width, height, 1, pixels.data(), width);
    } else {
        stbi_write_bmp(file_path.c_str(), width, height, 1, pixels.data());
    }
}