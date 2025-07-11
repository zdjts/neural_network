#include <iomanip>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#include "../core/CsvDataSet.cpp"
#include "../src/stb/LoadImage.h"
#include "../src/stb/SaveImage.h"
#include "CsvDataSet.h"
#include "Graph.h"
#include "ops.h"
// 辅助函数：获取预测结果的类别
int get_predicted_class(const Matrix& prediction_row) {
  int max_idx = 0;
  for (int i = 1; i < prediction_row.get_cols(); ++i) {
    if (prediction_row(0, i) > prediction_row(0, max_idx)) {
      max_idx = i;
    }
  }
  return max_idx;
}

void data_set(Matrix& source) {
  std::vector<float>& data = source.get_data();

  for (auto& i : data) {
    if (i > 0.1f)
      i = 1.0f;
    else
      i = 0.0f;
  }
}
int main(int argc, char** argv) {
  /*if (argc < 2) {*/
  /*  throw std::runtime_error("运行错误，传入参数数量不足\n");*/
  /*}*/

  Graph graph;
  float learning_rate = 0.05f;
  int epochs = 10;  // 对于大数据集，epoch可以少一些
  int batch_size = 64;
  int input_size = 784;
  int hidden_size = 32;
  int output_size = 10;

  LoadImage l_image = LoadImage(std::string(argv[1]), 1);

  /*LoadImage l_image{"../models/3.bmp"};*/

  Matrix x_m = l_image.get_Matrix();

  // data_set(x_m);
  auto x = graph.make_node(x_m, "X");

  auto W1 = graph.make_node(Matrix(input_size, hidden_size), "W1");
  auto b1 = graph.make_node(Matrix(1, hidden_size), "b1");
  auto W2 = graph.make_node(Matrix(hidden_size, output_size), "W2");
  auto b2 = graph.make_node(Matrix(1, output_size), "b2");

  std::map<std::string, std::shared_ptr<Node>> m{
      {"W1", W1}, {"b1", b1}, {"W2", W2}, {"b2", b2}};

  load_parameters("../models/MNIST_1_01.model", m);
  auto z1 = add_with_broadcast(graph, mul(graph, x, W1), b1);

  auto a1 = sigmoid(graph, z1);

  auto z2 = add_with_broadcast(graph, mul(graph, a1, W2), b2);
  z2->value = softmax(z2->value);
  for (int i = 0; i < z2->value.get_cols(); ++i) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << i << " :   " << z2->value(0, i) << std::endl;
  }
  SaveImage::save("new_load_image.bmp", x_m);
  return get_predicted_class(z2->value);

  return 0;
}
