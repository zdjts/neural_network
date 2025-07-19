#include "Graph.h"
// Graph.cpp
std::shared_ptr<Node> Graph::make_node(Matrix value, const std::string& name) {
  // 调用Node构造函数，让parents使用默认的空vector
  auto node = std::shared_ptr<Node>(new Node(std::move(value), {}, name));
  nodes_.push_back(node);
  return node;
}

std::shared_ptr<Node> Graph::make_node(
    Matrix value, const std::vector<std::weak_ptr<Node>>& parents,
    const std::string& name) {
  auto node = std::shared_ptr<Node>(new Node(std::move(value), parents, name));
  nodes_.push_back(node);
  return node;
}
