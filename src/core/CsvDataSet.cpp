
#include "CsvDataSet.h"

#include <Node.h>
#include <sys/types.h>

#include <cstdint>
#include <fstream>  // 用于文件输入流 std::ifstream
#include <ios>
#include <iostream>
#include <memory>
#include <sstream>    // 用于字符串流 std::stringstream
#include <stdexcept>  // 用于抛出异常 std::runtime_error
#include <string>
#include <utility>
#include <vector>

#include "Matrix.h"

// --- 公共构造函数 ---
// 它的职责很简单：调用私有的加载函数来完成所有工作。
CsvDataSet::CsvDataSet(const std::string& filepath, bool normalize)
    : CsvDataSet(load_from_file(filepath, normalize))  // 使用委托构造函数
{}

CsvDataSet::CsvDataSet(std::pair<Matrix, Matrix> p)
    : features_(p.first), labels_(p.second) {}
std::pair<Matrix, Matrix> CsvDataSet::load_from_file(const std::string& path,
                                                     bool normalize) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("csv文件无法加载， path = " + path + "\n");
  }
  std::vector<std::vector<float>> features_vec;
  std::vector<std::vector<float>> labels_vec;

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty()) continue;

    std::vector<float> feature_row;
    std::vector<float> label_row(1);
    std::stringstream ss(line);

    std::string value;
    if (std::getline(ss, value, ',')) {
      label_row[0] = std::stof(value);

    } else {
      continue;
    }
    while (std::getline(ss, value, ',')) {
      if (normalize) {
        feature_row.push_back(std::stof(value) / 255.0f);
      } else {
        feature_row.push_back(std::stof(value));
      }
    }
    if (feature_row.size() != 784) {
      throw std::runtime_error("csv 中，第" +
                               std::to_string(features_vec.size()) +
                               "行数量不等于784\n");
    }
    features_vec.push_back(feature_row);
    labels_vec.push_back(label_row);
  }
  return {Matrix(features_vec), Matrix(labels_vec)};
}

int CsvDataSet::size() const { return features_.get_rows(); }

// 获取整个数据集的引用
const Matrix& CsvDataSet::get_features() const { return features_; }

const Matrix& CsvDataSet::get_labels() const { return labels_; }

std::pair<Matrix, Matrix> CsvDataSet::get_item(int index) const {
  if (index < 0 || index >= features_.get_rows()) {
    throw std::runtime_error("index 错误 不够");
  }

  Matrix x(1, features_.get_cols()), y(1, 1);

  for (int i = 0; i < features_.get_cols(); ++i) {
    x(0, i) = features_(index, i);
  }
  y(0, 0) = labels_(index, 0);
  return {x, y};
}

void save_parameters(const std::string& filepath,
                     const std::vector<std::shared_ptr<Node>>& parameters) {
  std::ofstream outfile(filepath, std::ios::out | std::ios::binary);
  if (!outfile.is_open()) {
    throw std::runtime_error("无法写入文件: " + filepath);
  }

  // 写入文件头
  const uint32_t magic_number = 0x4D4E4E50;  // "MNNP"
  const uint32_t version = 1;
  const uint32_t param_count = static_cast<uint32_t>(parameters.size());
  outfile.write(reinterpret_cast<const char*>(&magic_number),
                sizeof(magic_number));
  outfile.write(reinterpret_cast<const char*>(&version), sizeof(version));
  outfile.write(reinterpret_cast<const char*>(&param_count),
                sizeof(param_count));

  // 依次写入每个参数
  for (const auto& param_node : parameters) {
    // 写入名称
    const std::string& name = param_node->name;
    uint32_t name_len = static_cast<uint32_t>(name.length());
    outfile.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));
    outfile.write(name.c_str(), name_len);

    // 写入维度
    const Matrix& mat = param_node->value;
    uint32_t rows = static_cast<uint32_t>(mat.get_rows());
    uint32_t cols = static_cast<uint32_t>(mat.get_cols());
    outfile.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
    outfile.write(reinterpret_cast<const char*>(&cols), sizeof(cols));

    // ✅ 效率优化：直接写入整个矩阵的数据块
    // 我们需要一个方法来访问Matrix内部的原始数据指针
    outfile.write(reinterpret_cast<const char*>(mat.get_data_ptr()),
                  rows * cols * sizeof(float));
  }
  outfile.close();
}

// =================================================================
// 加载参数
// =================================================================
void load_parameters(
    const std::string& filepath,
    std::map<std::string, std::shared_ptr<Node>>& parameters_map) {
  std::ifstream infile(filepath, std::ios::in | std::ios::binary);
  if (!infile.is_open()) {
    throw std::runtime_error("无法加载文件: " + filepath);
  }

  // 读取并验证文件头
  uint32_t magic_number, version, param_count;
  infile.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
  infile.read(reinterpret_cast<char*>(&version), sizeof(version));
  infile.read(reinterpret_cast<char*>(&param_count), sizeof(param_count));

  if (magic_number != 0x4D4E4E50 || version != 1) {
    throw std::runtime_error("文件格式不正确或版本不匹配。");
  }

  // 依次读取每个参数
  for (uint32_t i = 0; i < param_count; ++i) {
    // ✅ Bug修复：正确读取字符串
    uint32_t name_len;
    infile.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
    std::string name(name_len, '\0');  // 创建一个足够大的string
    infile.read(&name[0], name_len);   // 读取到string的内部缓冲区

    // ✅ Bug修复：正确检查key是否存在
    if (parameters_map.find(name) == parameters_map.end()) {
      std::cout << "警告：在模型文件中找到参数 '" << name
                << "', 但在提供的参数映射表中未找到，将跳过加载。" << std::endl;
      throw std::runtime_error("警告， key不存在\n");
      // 跳过这个参数的数据
      /*uint32_t rows, cols;*/
      /*infile.read(reinterpret_cast<char*>(&rows), sizeof(rows));*/
      /*infile.read(reinterpret_cast<char*>(&cols), sizeof(cols));*/
      /*infile.seekg(rows * cols * sizeof(float), std::ios_base::cur);*/
      /*continue;*/
    }

    // 获取对应的节点
    auto& param_node = parameters_map.at(name);

    // 读取并验证维度
    uint32_t rows, cols;
    infile.read(reinterpret_cast<char*>(&rows), sizeof(rows));
    infile.read(reinterpret_cast<char*>(&cols), sizeof(cols));
    if (param_node->value.get_rows() != rows ||
        param_node->value.get_cols() != cols) {
      throw std::runtime_error("参数 '" + name + "' 的维度不匹配。");
    }

    // ✅ Bug修复与效率优化：一次性读取整个数据块
    infile.read(reinterpret_cast<char*>(param_node->value.get_data_ptr()),
                rows * cols * sizeof(float));
  }
  infile.close();
}
