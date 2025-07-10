#ifndef OPS_H
#define OPS_H

#include <memory>

#include "../core/Graph.h"
#include "../core/Matrix.h"

// 生明加法运算函数
std::shared_ptr<Node> add(Graph& graph, std::shared_ptr<Node> a,
                          std::shared_ptr<Node> b);

std::shared_ptr<Node> mul(Graph& graph, std::shared_ptr<Node> a,
                          std::shared_ptr<Node> b);

std::shared_ptr<Node> sum(Graph& graph, std::shared_ptr<Node> a);

std::shared_ptr<Node> relu(Graph& graph, std::shared_ptr<Node> a);

std::shared_ptr<Node> sub(Graph& graph, std::shared_ptr<Node> a,
                          std::shared_ptr<Node> b);

std::shared_ptr<Node> mse_loss(Graph& graph,
                               const std::shared_ptr<Node> prediction,
                               const std::shared_ptr<Node> target);

std::shared_ptr<Node> element_mul(Graph& graph, std::shared_ptr<Node> a,
                                  std::shared_ptr<Node> b);

std::shared_ptr<Node> add_with_broadcast(Graph& graph,
                                         const std::shared_ptr<Node>& a,
                                         const std::shared_ptr<Node>& b);
Matrix one_hot_encode(const Matrix& Y_labels, int num_classes = 10);

Matrix softmax(const Matrix other);

std::shared_ptr<Node> softmax_cross_entropy_loss(Graph& graph,
                                                 std::shared_ptr<Node> logits,
                                                 std::shared_ptr<Node> target);
#endif  // !OPS_H
