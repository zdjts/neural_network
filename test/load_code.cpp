#include <map>
#include <memory>
#include <string>

#include "../core/CsvDataSet.cpp"
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

int main() {
  Graph g;

  /*float learning_rate = 0.05f;*/
  /*int epochs = 25;  // 对于大数据集，epoch可以少一些*/
  /*int batch_size = 64;*/
  int input_size = 784;
  int hidden_size = 128;
  int output_size = 10;
  auto W1 = g.make_node(Matrix(input_size, hidden_size), "W1");
  auto b1 = g.make_node(Matrix(1, hidden_size), "b1");
  auto W2 = g.make_node(Matrix(hidden_size, output_size), "W2");
  auto b2 = g.make_node(Matrix(1, output_size), "b2");

  std::map<std::string, std::shared_ptr<Node>> m{
      {"W1", W1}, {"b1", b1}, {"W2", W2}, {"b2", b2}};
  load_parameters("../models/MNIST_1.model", m);

  CsvDataSet c("../data/mnist_test.csv");

  auto x = g.make_node(c.get_features(), "X_test");
  auto y = g.make_node(c.get_labels(), "Y_test");
  auto z1 = add_with_broadcast(g, mul(g, x, W1), b1);
  auto a1 = relu(g, z1);
  auto z2 = add_with_broadcast(g, mul(g, a1, W2), b2);

  int sum = 0;

  for (int i = 0; i < c.size(); ++i) {
    Matrix s = z2->value.get_row(i);
    int yv_y = get_predicted_class(s);
    int la_y = y->value(i, 0);

    if (yv_y == la_y) sum++;
  }

  float jieguo = static_cast<float>(sum) / static_cast<float>(c.size());
  std::cout << "预测结果成功率为" << jieguo << std::endl;
}
