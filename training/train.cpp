#include <algorithm>  
#include <cmath>
#include <iostream>
#include <memory>
#include <numeric>  
#include <random>
#include <vector>

#include "../src/core/CsvDataSet.h"
#include "../src/core/Graph.h"
#include "../src/core/Matrix.h"
#include "../src/ops/noise.h"  
#include "../src/ops/ops.h"


Matrix random_matrix(int rows, int cols) {
  Matrix m(rows, cols);
  std::mt19937 gen(49);  
  std::uniform_real_distribution<> distr(-0.5, 0.5);
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      m(i, j) = static_cast<float>(distr(gen));
    }
  }
  return m;
}


int get_predicted_class(const Matrix& prediction_row) {
  int max_idx = 0;
  for (int i = 1; i < prediction_row.get_cols(); ++i) {
    if (prediction_row(0, i) > prediction_row(0, max_idx)) {
      max_idx = i;
    }
  }
  return max_idx;
}


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
    
    std::cout << "Loading data..." << std::endl;
    CsvDataSet train_dataset("../data/mnist_train.csv");
    CsvDataSet test_dataset("../data/mnist_test.csv");

    Matrix X_train_full = train_dataset.get_features();
    Matrix Y_train_full = one_hot_encode(train_dataset.get_labels());
    Matrix X_test_full = test_dataset.get_features();

    std::cout << "Training data loaded: " << train_dataset.size() << " samples."
              << std::endl;
    std::cout << "Test data loaded: " << test_dataset.size() << " samples."
              << std::endl;

    
    Graph g;
    float learning_rate = 0.05f;
    int epochs = 10;
    int batch_size = 64;
    int input_size = 784;
    int hidden_size = 128;
    int output_size = 10;

    
    auto W1 = g.make_node(random_matrix(input_size, hidden_size), "W1");
    auto b1 = g.make_node(Matrix(1, hidden_size, 0.01f), "b1");
    auto W2 = g.make_node(random_matrix(hidden_size, output_size), "W2");
    auto b2 = g.make_node(Matrix(1, output_size, 0.0f), "b2");

    
    std::cout << "Training started..." << std::endl;
    int num_samples = train_dataset.size();
    std::vector<int> indices(num_samples);
    std::iota(indices.begin(), indices.end(), 0);
    std::mt19937 rng(59);

    for (int i = 0; i < epochs; ++i) {
      std::shuffle(indices.begin(), indices.end(), rng);

      double epoch_loss = 0.0;
      int num_batches = 0;

      for (int start_idx = 0; start_idx < num_samples;
           start_idx += batch_size) {
        int end_idx = std::min(start_idx + batch_size, num_samples);
        std::vector<int> batch_indices(indices.begin() + start_idx,
                                       indices.begin() + end_idx);

        Matrix original_batch = create_batch(X_train_full, batch_indices);

        
        auto X_batch = g.make_node(original_batch, "X_batch");
        auto Y_batch =
            g.make_node(create_batch(Y_train_full, batch_indices), "Y_batch");

        
        W1->grad.clear_grad();
        b1->grad.clear_grad();
        W2->grad.clear_grad();
        b2->grad.clear_grad();

        
        auto z1 = add_with_broadcast(g, mul(g, X_batch, W1), b1);
        auto a1 = sigmoid(g, z1);
        auto logits = add_with_broadcast(g, mul(g, a1, W2), b2);

        
        auto loss = softmax_cross_entropy_loss(g, logits, Y_batch);
        epoch_loss += loss->value(0, 0);

        
        loss->backward();

        
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

    
    std::cout << "\n--- Evaluating on Test Set ---" << std::endl;
    auto X_test = g.make_node(test_dataset.get_features(), "X_test");
    auto Y_test_labels = test_dataset.get_labels();

    auto z1 = add_with_broadcast(g, mul(g, X_test, W1), b1);
    auto a1 = sigmoid(g, z1);
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

    
    std::string save_path = "../models/MNIST_1_sigmoid.model";
    save_parameters(save_path, {W1, b1, W2, b2});
    std::cout << "Model saved to " << save_path << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "An error occurred: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
