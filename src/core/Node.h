#ifndef NODE_H
#define NODE_H
#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Matrix.h"

class Graph;
class Node : public std::enable_shared_from_this<Node> {
 private:
  friend class Graph;
  // 将构造函数私有化，并禁用拷贝、移动函数，使得所有的Node节点都被友元类Graph管理、创建
  explicit Node(Matrix v, const std::vector<std::weak_ptr<Node>>& p,
                const std::string& name);

 public:
  Matrix value;
  Matrix grad;
  const std::vector<std::weak_ptr<Node>> parent;
  std::function<void()> _backward;
  std::string name;

  // --- 禁用拷贝和移动，保证节点身份唯一 ---
  Node(const Node&) = delete;
  Node& operator=(const Node&) = delete;
  Node(Node&&) = delete;
  Node& operator=(Node&&) = delete;

  // --- 公共方法 ---
  void backward();
};
#endif  // !NODE_H
