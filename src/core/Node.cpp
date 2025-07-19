#include "Node.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "Matrix.h"

Node::Node(Matrix v, const std::vector<std::weak_ptr<Node>>& p,
           const std::string& n)
    : value(std::move(v)),
      grad(Matrix::zeros(value.get_rows(), value.get_cols())),
      parent(p),
      _backward([]() {}),
      name(n) {}

void Node::backward() {
  std::vector<Node*> topo_order;
  std::set<Node*> visited;

  std::function<void(Node*)> build_topo = [&](Node* node) {
    visited.insert(node);
    for (auto i : node->parent) {
      if (auto j = i.lock()) {
        if (visited.find(j.get()) == visited.end()) {
          build_topo(j.get());
        }
      }
    }
    topo_order.push_back(node);
  };

  build_topo(this);

  this->grad = Matrix(1, 1, 1.0f);

  std::reverse(topo_order.begin(), topo_order.end());
  for (Node* node : topo_order) node->_backward();
}
