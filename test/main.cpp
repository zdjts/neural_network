#include <algorithm>  // For std::shuffle
#include <cmath>
#include <iostream>
#include <memory>
#include <numeric>  // For std::iota
#include <random>
#include <vector>

#include "../src/core/CsvDataSet.h"
#include "../src/core/Graph.h"
#include "../src/core/Matrix.h"
#include "../src/ops/ops.h"

// 辅助函数：随机初始化权重
Matrix random_matrix(int rows, int cols) {
  Matrix m(rows, cols);
  std::mt19937 gen(42);  // 使用固定种子以保证结果可复现
  std::uniform_real_distribution<> distr(-0.5, 0.5);
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      m(i, j) = static_cast<float>(distr(gen));
    }
  }
  return m;
}

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

// 新的辅助函数：根据索引创建批次数据
Matrix create_batch(const Matrix& source, const std::vector<int>& indices) {
  Matrix batch(indices.size(), source.get_cols());
  for (size_t i = 0; i < indices.size(); ++i) {
    for (int j = 0; j < source.get_cols(); ++j) {
      batch(i, j) = source(indices[i], j);
    }
  }
  return batch;
}

int main() {
  try {
    // --- 1. 数据准备 ---
    std::cout << "Loading data..." << std::endl;
    // 使用你提供的文件路径结构，但加载完整的数据集
    CsvDataSet train_dataset("../data/mnist_train.csv");
    CsvDataSet test_dataset("../data/mnist_test.csv");

    Matrix X_train_full = train_dataset.get_features();
    Matrix Y_train_full = one_hot_encode(train_dataset.get_labels());

    std::cout << "Training data loaded: " << train_dataset.size() << " samples."
              << std::endl;
    std::cout << "Test data loaded: " << test_dataset.size() << " samples."
              << std::endl;

    // --- 2. 初始化网络和超参数 ---
    Graph g;
    float learning_rate = 0.05f;
    int epochs = 10;  // 对于大数据集，epoch可以少一些
    int batch_size = 64;
    int input_size = 784;
    int hidden_size = 128;
    int output_size = 10;

    // --- 3. 定义网络参数 ---
    auto W1 = g.make_node(random_matrix(input_size, hidden_size), "W1");
    auto b1 = g.make_node(Matrix(1, hidden_size, 0.01f), "b1");
    auto W2 = g.make_node(random_matrix(hidden_size, output_size), "W2");
    auto b2 = g.make_node(Matrix(1, output_size, 0.0f), "b2");

    // --- 4. 训练循环 ---
    std::cout << "Training started..." << std::endl;
    int num_samples = train_dataset.size();
    std::vector<int> indices(num_samples);
    std::iota(indices.begin(), indices.end(), 0);  // 生成 0, 1, ..., N-1
    std::mt19937 rng(42);  // 创建一个用于打乱的随机数引擎

    for (int i = 0; i < epochs; ++i) {
      // 在每个epoch开始时打乱数据
      std::shuffle(indices.begin(), indices.end(), rng);

      double epoch_loss = 0.0;
      int num_batches = 0;
      // 按批次进行训练
      for (int start_idx = 0; start_idx < num_samples;
           start_idx += batch_size) {
        int end_idx = std::min(start_idx + batch_size, num_samples);

        std::vector<int> batch_indices(indices.begin() + start_idx,
                                       indices.begin() + end_idx);

        // 创建当前批次的数据节点
        auto X_batch =
            g.make_node(create_batch(X_train_full, batch_indices), "X_batch");
        auto Y_batch =
            g.make_node(create_batch(Y_train_full, batch_indices), "Y_batch");

        // 清零梯度
        W1->grad.clear_grad();
        b1->grad.clear_grad();
        W2->grad.clear_grad();
        b2->grad.clear_grad();

        // 前向传播
        auto z1 = add_with_broadcast(g, mul(g, X_batch, W1), b1);
        auto a1 = relu(g, z1);
        auto logits = add_with_broadcast(g, mul(g, a1, W2), b2);

        // 计算损失
        auto loss = softmax_cross_entropy_loss(g, logits, Y_batch);
        epoch_loss += loss->value(0, 0);

        // 反向传播
        loss->backward();

        // 更新权重
        W1->value = W1->value - (W1->grad * learning_rate);
        b1->value = b1->value - (b1->grad * learning_rate);
        W2->value = W2->value - (W2->grad * learning_rate);
        b2->value = b2->value - (b2->grad * learning_rate);

        num_batches++;
      }
      std::cout << "Epoch " << i
                << ", Average Loss: " << epoch_loss / num_batches << std::endl;
    }
    std::cout << "Training finished!" << std::endl;

    // --- 5. 在测试集上进行最终评估 ---
    std::cout << "\n--- Evaluating on Test Set ---" << std::endl;
    auto X_test = g.make_node(test_dataset.get_features(), "X_test");
    auto Y_test_labels = test_dataset.get_labels();

    auto z1 = add_with_broadcast(g, mul(g, X_test, W1), b1);
    auto a1 = relu(g, z1);
    auto final_logits = add_with_broadcast(g, mul(g, a1, W2), b2);

    int correct_predictions = 0;
    for (int i = 0; i < test_dataset.size(); i++) {
      Matrix single_prediction_logits = final_logits->value.get_row(i);
      int predicted_class = get_predicted_class(single_prediction_logits);
      int actual_class = static_cast<int>(Y_test_labels(i, 0));
      if (predicted_class == actual_class) {
        correct_predictions++;
      }
    }

    double accuracy =
        static_cast<double>(correct_predictions) / test_dataset.size() * 100.0;
    std::cout << "\nFinal Test Accuracy: " << accuracy << "%" << std::endl;

    // 数据保存在文件中
    std::string save_path = "../MNIST_1.model";
    save_parameters(save_path, {W1, b1, W2, b2});
  } catch (const std::exception& e) {
    std::cerr << "An error occurred: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
