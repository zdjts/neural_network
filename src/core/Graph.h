#ifndef GRAPH_H
#define GRAPH_H
#include <memory>

#include "Node.h"

class Graph {
 private:
  std::vector<std::shared_ptr<Node>> nodes_;

 public:
  Graph() = default;

  std::shared_ptr<Node> make_node(Matrix value, const std::string& name);
  std::shared_ptr<Node> make_node(
      Matrix value, const std::vector<std::weak_ptr<Node>>& parents,
      const std::string& name);
};

#endif  // !GRAPH_H
