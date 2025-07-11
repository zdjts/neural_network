#include "ops.h"

#include <cmath>
#include <memory>
#include <utility>
#include <vector>

#include "Graph.h"
#include "Matrix.h"

std::shared_ptr<Node> add(Graph &graph, std::shared_ptr<Node> a,
                          std::shared_ptr<Node> b) {
  Matrix c_value = a->value + b->value;
  std::vector<std::weak_ptr<Node>> parents = {a, b};

  auto c = graph.make_node(std::move(c_value), parents, "add");

  // 反向传播
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
  // 前向传播：计算 Sigmoid 的输出值
  int rows = a->value.get_rows();
  int cols = a->value.get_cols();
  Matrix m(rows, cols);  // 用于存储 Sigmoid 的输出结果

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      // 计算 Sigmoid(x) = 1 / (1 + e^(-x))
      m(i, j) = 1.0f / (1.0f + std::exp(-a->value(i, j)));
    }
  }

  // 创建新的 Node 节点来存储 Sigmoid 的输出
  std::vector<std::weak_ptr<Node>> parents;
  parents.push_back(a);
  std::shared_ptr<Node> c =
      graph.make_node(m, parents, "Sigmoid");  // 节点名称为 "Sigmoid"

  // 定义反向传播逻辑
  std::weak_ptr<Node> c_weak = c;  // 使用 weak_ptr 防止循环引用

  c->_backward = [a, c_weak]() {          // 捕获上游节点 a 和当前节点 c_weak
    if (auto c_shared = c_weak.lock()) {  // 锁定 weak_ptr 以安全访问 c
      Matrix upstream_grad = c_shared->grad;  // 从当前节点 c 获取上游梯度

      // 计算 Sigmoid 的局部梯度：sigma'(x) = sigma(x) * (1 - sigma(x))
      // 注意：这里的 sigma(x) 是前向传播时 c->value 中的值
      Matrix local_grad(a->value.get_rows(), a->value.get_cols());
      for (int i = 0; i < local_grad.get_rows(); ++i) {
        for (int j = 0; j < local_grad.get_cols(); ++j) {
          float sigmoid_val =
              c_shared->value(i, j);  // 获取前向传播时 Sigmoid 的输出值
          local_grad(i, j) = sigmoid_val * (1.0f - sigmoid_val);
        }
      }

      // 将上游梯度与局部梯度进行元素级乘法，并累加到输入节点 a 的梯度中
      a->grad = a->grad + upstream_grad.element_mul(local_grad);
    }
  };
  return c;  // 返回 Sigmoid 节点的 shared_ptr
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

      // 创建一个和输入同维度的矩阵来存放局部梯度
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

// 逐元素乘法 (最终修正版)
std::shared_ptr<Node> element_mul(Graph &graph, std::shared_ptr<Node> a,
                                  std::shared_ptr<Node> b) {
  Matrix result_value = a->value.element_mul(b->value);
  auto c = graph.make_node(std::move(result_value), {a, b}, "element_mul");

  std::weak_ptr<Node> c_weak = c;
  c->_backward = [a, b, c_weak]() {
    if (auto c_shared = c_weak.lock()) {
      auto upstream_grad = c_shared->grad;
      // ✅ 关键修正: 检查a和b是否是同一个节点
      if (a == b) {
        // 如果是自乘 (A*A), 梯度是 2 * grad * A
        a->grad = a->grad + (upstream_grad.element_mul(a->value) * 2.0f);
      } else {
        // 如果是不同节点相乘
        a->grad = a->grad + upstream_grad.element_mul(b->value);
        b->grad = b->grad + upstream_grad.element_mul(a->value);
      }
    }
  };
  return c;
}

// ... 其他函数 (sum, mul, relu, add_with_broadcast, mse_loss) ...

std::shared_ptr<Node> mse_loss(Graph &graph,
                               const std::shared_ptr<Node> prediction,
                               const std::shared_ptr<Node> target) {
  auto error = sub(graph, prediction, target);

  auto squared_error = element_mul(graph, error, error);

  auto loss = sum(graph, squared_error);

  loss->name = "mse_loss";  // 可以给最终的loss节点起个有意义的名字
  return loss;
}

// 减法运算 (经单元测试验证)
// ✅ 修正: 将参数改为按值传递，以匹配链接器期望的函数签名
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
    const std::shared_ptr<Node> &a,    // M x N 矩阵
    const std::shared_ptr<Node> &b) {  // 1 x N 向量
  // --- 前向传播检查 ---
  if (a->value.get_cols() != b->value.get_cols() || b->value.get_rows() != 1) {
    throw std::invalid_argument(
        "Dimension mismatch for add_with_broadcast. Expected (M,N) + (1,N).");
  }

  // --- 前向传播计算 ---
  Matrix result_value = a->value;  // 先拷贝一份矩阵a的值
  // 循环将向量b的每个元素加到矩阵a的对应列的每个元素上
  for (int i = 0; i < a->value.get_rows(); ++i) {
    for (int j = 0; j < a->value.get_cols(); ++j) {
      result_value(i, j) += b->value(0, j);
    }
  }

  // --- 创建结果节点 ---
  auto c = graph.make_node(std::move(result_value), {a, b}, "add_broadcast");

  // --- 定义反向传播逻辑 ---
  std::weak_ptr<Node> c_weak = c;
  c->_backward = [a, b, c_weak]() {
    if (auto c_shared = c_weak.lock()) {
      // 对于矩阵a，梯度直接传递回去，维度不变
      a->grad = a->grad + c_shared->grad;

      // 对于向量b，需要将上游梯度的所有行相加，实现梯度汇总
      // 这是广播的反向操作
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

// Loss = -sum(targets * log(P))
std::shared_ptr<Node> softmax_cross_entropy_loss(
    Graph &graph, std::shared_ptr<Node> logits, std::shared_ptr<Node> targets) {
  // --- 1. 前向传播 ---

  // a) 计算Softmax概率 (P)
  // 为了数值稳定性，我们使用 "max-subtraction trick"
  // 即：softmax(x) = softmax(x - max(x))
  Matrix logits_val = logits->value;
  Matrix probabilities = Matrix(logits_val.get_rows(), logits_val.get_cols());

  for (int i = 0; i < logits_val.get_rows(); ++i) {
    // 找到当前行的最大值
    float max_logit = logits_val(i, 0);
    for (int j = 1; j < logits_val.get_cols(); ++j) {
      if (logits_val(i, j) > max_logit) {
        max_logit = logits_val(i, j);
      }
    }

    // 计算exp(logit - max)的总和
    float sum_exp = 0.0f;
    for (int j = 0; j < logits_val.get_cols(); ++j) {
      sum_exp += std::exp(logits_val(i, j) - max_logit);
    }

    // 计算最终的softmax概率
    for (int j = 0; j < logits_val.get_cols(); ++j) {
      probabilities(i, j) = std::exp(logits_val(i, j) - max_logit) / sum_exp;
    }
  }

  // b) 计算交叉熵损失
  // Loss = -sum(targets * log(P))
  float total_loss = 0.0f;
  for (int i = 0; i < probabilities.get_rows(); ++i) {
    for (int j = 0; j < probabilities.get_cols(); ++j) {
      // 检查 targets_one_hot(i,j) 是否为1
      if (targets->value(i, j) == 1.0f) {
        // 加上一个很小的数epsilon防止log(0)导致NaN
        total_loss -= std::log(probabilities(i, j) + 1e-9);
      }
    }
  }
  // 对整个批次的loss求平均
  total_loss /= static_cast<float>(probabilities.get_rows());

  Matrix loss_matrix(1, 1, total_loss);

  // --- 2. 创建结果节点 ---
  auto loss_node = graph.make_node(std::move(loss_matrix), {logits, targets},
                                   "softmax_cross_entropy_loss");

  // --- 3. 定义反向传播逻辑 ---
  std::weak_ptr<Node> loss_node_weak = loss_node;
  // 将前向计算出的概率矩阵捕获到lambda中
  loss_node->_backward = [logits, targets, probabilities, loss_node_weak]() {
    if (auto loss_shared = loss_node_weak.lock()) {
      // 梯度公式异常简单: (P - T) / N
      Matrix grad = (probabilities - targets->value) *
                    (1.0f / static_cast<float>(probabilities.get_rows()));

      // 累加梯度到logits节点
      logits->grad = logits->grad + grad;
    }
  };

  return loss_node;
}
