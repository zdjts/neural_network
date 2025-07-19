#include "ops.h"

#include <cmath>
#include <memory>
#include <utility>
#include <vector>

#include "../core/Graph.h"
#include "../core/Matrix.h"

std::shared_ptr<Node> add(Graph &graph, std::shared_ptr<Node> a,
                          std::shared_ptr<Node> b) {
  Matrix c_value = a->value + b->value;
  std::vector<std::weak_ptr<Node>> parents = {a, b};

  auto c = graph.make_node(std::move(c_value), parents, "add");

  
  std::weak_ptr<Node> c_weak = c;

  c->_backward = [a, b, c_weak]() {
    if (auto c_shared = c_weak.lock()) {
      a->grad = a->grad + c_shared->grad;
      b->grad = b->grad + c_shared->grad;
    }
  };
  return c;
}

std::shared_ptr<Node> sum(Graph &graph, const std::shared_ptr<Node> a) {
  Matrix m = a->value.sum();
  std::vector<std::weak_ptr<Node>> parents = {a};
  std::shared_ptr<Node> c = graph.make_node(m, parents, "sum");

  std::weak_ptr<Node> c_weak = c;

  c->_backward = [a, c_weak]() {
    if (auto c_shared = c_weak.lock()) {
      float upstream_grad_val = c_shared->grad(0, 0);

      Matrix grad_to_propagate(a->value.get_rows(), a->value.get_cols(),
                               upstream_grad_val);

      a->grad = a->grad + grad_to_propagate;
    }
  };
  return c;
}
std::shared_ptr<Node> mul(Graph &graph, std::shared_ptr<Node> a,
                          std::shared_ptr<Node> b) {
  Matrix c_value = a->value * b->value;
  std::vector<std::weak_ptr<Node>> parents = {a, b};

  auto c = graph.make_node(std::move(c_value), parents, "mul");

  std::weak_ptr<Node> c_weak = c;

  c->_backward = [a, b, c_weak]() {
    if (auto c_shared = c_weak.lock()) {
      a->grad = a->grad + c_shared->grad * b->value.transpose();
      b->grad = b->grad + a->value.transpose() * c_shared->grad;
    }
  };
  return c;
}

std::shared_ptr<Node> sigmoid(Graph &graph, std::shared_ptr<Node> a) {
  
  int rows = a->value.get_rows();
  int cols = a->value.get_cols();
  Matrix m(rows, cols);  

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      
      m(i, j) = 1.0f / (1.0f + std::exp(-a->value(i, j)));
    }
  }

  
  std::vector<std::weak_ptr<Node>> parents;
  parents.push_back(a);
  std::shared_ptr<Node> c =
      graph.make_node(m, parents, "Sigmoid");  

  
  std::weak_ptr<Node> c_weak = c;  

  c->_backward = [a, c_weak]() {          
    if (auto c_shared = c_weak.lock()) {  
      Matrix upstream_grad = c_shared->grad;  

      
      
      Matrix local_grad(a->value.get_rows(), a->value.get_cols());
      for (int i = 0; i < local_grad.get_rows(); ++i) {
        for (int j = 0; j < local_grad.get_cols(); ++j) {
          float sigmoid_val =
              c_shared->value(i, j);  
          local_grad(i, j) = sigmoid_val * (1.0f - sigmoid_val);
        }
      }

      
      a->grad = a->grad + upstream_grad.element_mul(local_grad);
    }
  };
  return c;  
}
std::shared_ptr<Node> relu(Graph &graph, std::shared_ptr<Node> a) {
  int rows = a->value.get_rows();
  int cols = a->value.get_cols();
  Matrix m(rows, cols);
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      m(i, j) = std::fmax(0.0f, a->value(i, j));
    }
  }
  std::vector<std::weak_ptr<Node>> parents;
  parents.push_back(a);
  std::shared_ptr<Node> c = graph.make_node(m, parents, "Relu");

  std::weak_ptr<Node> c_weak = c;
  c->_backward = [a, c_weak]() {
    if (auto c_shared = c_weak.lock()) {
      Matrix upstream_grad = c_shared->grad;

      
      Matrix local_grad(a->value.get_rows(), a->value.get_cols());
      for (int i = 0; i < local_grad.get_rows(); ++i) {
        for (int j = 0; j < local_grad.get_cols(); ++j) {
          local_grad(i, j) = (a->value(i, j) > 0.0f) ? 1.0f : 0.0f;
        }
      }

      a->grad = a->grad + upstream_grad.element_mul(local_grad);
    }
  };
  return c;
}


std::shared_ptr<Node> element_mul(Graph &graph, std::shared_ptr<Node> a,
                                  std::shared_ptr<Node> b) {
  Matrix result_value = a->value.element_mul(b->value);
  auto c = graph.make_node(std::move(result_value), {a, b}, "element_mul");

  std::weak_ptr<Node> c_weak = c;
  c->_backward = [a, b, c_weak]() {
    if (auto c_shared = c_weak.lock()) {
      auto upstream_grad = c_shared->grad;
      if (a == b) {
        
        a->grad = a->grad + (upstream_grad.element_mul(a->value) * 2.0f);
      } else {
        
        a->grad = a->grad + upstream_grad.element_mul(b->value);
        b->grad = b->grad + upstream_grad.element_mul(a->value);
      }
    }
  };
  return c;
}



std::shared_ptr<Node> mse_loss(Graph &graph,
                               const std::shared_ptr<Node> prediction,
                               const std::shared_ptr<Node> target) {
  auto error = sub(graph, prediction, target);

  auto squared_error = element_mul(graph, error, error);

  auto loss = sum(graph, squared_error);

  loss->name = "mse_loss";  
  return loss;
}


std::shared_ptr<Node> sub(Graph &graph, std::shared_ptr<Node> a,
                          std::shared_ptr<Node> b) {
  Matrix result_value = a->value - b->value;
  auto c = graph.make_node(std::move(result_value), {a, b}, "sub");

  std::weak_ptr<Node> c_weak = c;
  c->_backward = [a, b, c_weak]() {
    if (auto c_shared = c_weak.lock()) {
      a->grad = a->grad + c_shared->grad;
      b->grad = b->grad - c_shared->grad;
    }
  };
  return c;
}
std::shared_ptr<Node> add_with_broadcast(
    Graph &graph,
    const std::shared_ptr<Node> &a,    
    const std::shared_ptr<Node> &b) {  
  
  if (a->value.get_cols() != b->value.get_cols() || b->value.get_rows() != 1) {
    throw std::invalid_argument(
        "Dimension mismatch for add_with_broadcast. Expected (M,N) + (1,N).");
  }

  
  Matrix result_value = a->value;  
  
  for (int i = 0; i < a->value.get_rows(); ++i) {
    for (int j = 0; j < a->value.get_cols(); ++j) {
      result_value(i, j) += b->value(0, j);
    }
  }

  
  auto c = graph.make_node(std::move(result_value), {a, b}, "add_broadcast");

  
  std::weak_ptr<Node> c_weak = c;
  c->_backward = [a, b, c_weak]() {
    if (auto c_shared = c_weak.lock()) {
      
      a->grad = a->grad + c_shared->grad;

      
      
      b->grad = b->grad + c_shared->grad.sum_rows();
    }
  };

  return c;
}

Matrix one_hot_encode(const Matrix &Y_labels, int num_classes) {
  Matrix m(Y_labels.get_rows(), num_classes, 0.0f);

  for (int i = 0; i < Y_labels.get_rows(); ++i) {
    m(i, Y_labels(i, 0)) = 1.0f;
  }

  return m;
}

Matrix softmax(const Matrix other) {
  Matrix m(other.get_rows(), other.get_cols());
  for (int i = 0; i < other.get_rows(); ++i) {
    float num = 0;

    for (int j = 0; j < other.get_cols(); ++j) {
      num += exp(other(i, j));
    }

    for (int j = 0; j < other.get_cols(); ++j) {
      m(i, j) = exp(other(i, j)) / num;
    }
  }
  return m;
}


std::shared_ptr<Node> softmax_cross_entropy_loss(
    Graph &graph, std::shared_ptr<Node> logits, std::shared_ptr<Node> targets) {
  
  Matrix logits_val = logits->value;
  Matrix probabilities = Matrix(logits_val.get_rows(), logits_val.get_cols());

  for (int i = 0; i < logits_val.get_rows(); ++i) {
    
    float max_logit = logits_val(i, 0);
    for (int j = 1; j < logits_val.get_cols(); ++j) {
      if (logits_val(i, j) > max_logit) {
        max_logit = logits_val(i, j);
      }
    }

    
    float sum_exp = 0.0f;
    for (int j = 0; j < logits_val.get_cols(); ++j) {
      sum_exp += std::exp(logits_val(i, j) - max_logit);
    }

    
    for (int j = 0; j < logits_val.get_cols(); ++j) {
      probabilities(i, j) = std::exp(logits_val(i, j) - max_logit) / sum_exp;
    }
  }

  
  
  float total_loss = 0.0f;
  for (int i = 0; i < probabilities.get_rows(); ++i) {
    for (int j = 0; j < probabilities.get_cols(); ++j) {
      
      if (targets->value(i, j) == 1.0f) {
        
        total_loss -= std::log(probabilities(i, j) + 1e-9);
      }
    }
  }
  
  total_loss /= static_cast<float>(probabilities.get_rows());

  Matrix loss_matrix(1, 1, total_loss);

  
  auto loss_node = graph.make_node(std::move(loss_matrix), {logits, targets},
                                   "softmax_cross_entropy_loss");

  
  std::weak_ptr<Node> loss_node_weak = loss_node;
  
  loss_node->_backward = [logits, targets, probabilities, loss_node_weak]() {
    if (auto loss_shared = loss_node_weak.lock()) {
      
      Matrix grad = (probabilities - targets->value) *
                    (1.0f / static_cast<float>(probabilities.get_rows()));

      
      logits->grad = logits->grad + grad;
    }
  };

  return loss_node;
}
