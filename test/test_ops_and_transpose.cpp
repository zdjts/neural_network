#include <cmath>
#include <iostream>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

// 通过相对路径包含项目头文件
#include "../core/Graph.h"
#include "../core/Matrix.h"
#include "../ops/ops.h"

// ===================================================================
//  区块 1: 测试辅助工具
// ===================================================================

// 辅助函数：打印测试结果
void print_test_result(const std::string& test_name, bool passed) {
  std::cout << "[" << (passed ? "PASS" : "\x1B[31mFAIL\x1B[0m")
            << "] "  // 使用ANSI颜色高亮失败的测试
            << test_name << std::endl;
}

// 辅助函数：比较两个浮点数是否近似相等
bool are_nearly_equal(float a, float b, float epsilon = 1e-6f) {
  return std::abs(a - b) < epsilon;
}

// 辅助函数：比较两个矩阵是否近似相等
bool compare_matrices(const Matrix& m1, const Matrix& m2,
                      float epsilon = 1e-6f) {
  if (m1.get_rows() != m2.get_rows() || m1.get_cols() != m2.get_cols()) {
    std::cerr << "  [Error] Dimension mismatch: (" << m1.get_rows() << "x"
              << m1.get_cols() << ") vs (" << m2.get_rows() << "x"
              << m2.get_cols() << ")\n";
    return false;
  }
  for (int i = 0; i < m1.get_rows(); ++i) {
    for (int j = 0; j < m1.get_cols(); ++j) {
      if (!are_nearly_equal(m1(i, j), m2(i, j), epsilon)) {
        std::cerr << "  [Error] Element mismatch at (" << i << "," << j
                  << "): " << m1(i, j) << " != " << m2(i, j) << "\n";
        return false;
      }
    }
  }
  return true;
}

// ===================================================================
//  区块 2: 测试主函数
// ===================================================================

int main() {
  bool all_tests_passed = true;

  // --- 1. Matrix::transpose() 测试 ---
  std::cout << "\n--- Running Matrix::transpose() Test Suite ---\n";
  {
    // 1.1 方阵转置
    std::string test_name = "Transpose: Square Matrix (2x2)";
    bool passed = false;
    try {
      Matrix m1({{1, 2}, {3, 4}});
      Matrix m1_T_expected({{1, 3}, {2, 4}});
      Matrix m1_T = m1.transpose();
      passed = compare_matrices(m1_T, m1_T_expected);
    } catch (const std::exception& e) {
      passed = false;
      std::cerr << e.what() << '\n';
    }
    print_test_result(test_name, passed);
    if (!passed) all_tests_passed = false;

    // 1.2 矩形转置
    test_name = "Transpose: Rectangular Matrix (2x3)";
    passed = false;
    try {
      Matrix m2({{1, 2, 3}, {4, 5, 6}});               // 2x3
      Matrix m2_T_expected({{1, 4}, {2, 5}, {3, 6}});  // 3x2
      Matrix m2_T = m2.transpose();
      passed = compare_matrices(m2_T, m2_T_expected);
    } catch (const std::exception& e) {
      passed = false;
      std::cerr << e.what() << '\n';
    }
    print_test_result(test_name, passed);
    if (!passed) all_tests_passed = false;

    // 1.3 行向量转置
    test_name = "Transpose: Row Vector (1x4)";
    passed = false;
    try {
      Matrix m3({{1, 2, 3, 4}});                   // 1x4
      Matrix m3_T_expected({{1}, {2}, {3}, {4}});  // 4x1
      Matrix m3_T = m3.transpose();
      passed = compare_matrices(m3_T, m3_T_expected);
    } catch (const std::exception& e) {
      passed = false;
      std::cerr << e.what() << '\n';
    }
    print_test_result(test_name, passed);
    if (!passed) all_tests_passed = false;
  }

  // --- 2. 运算与计算图测试 (前向传播) ---
  std::cout << "\n--- Running Ops Forward Pass Test Suite ---\n";
  Graph graph;

  // --- 【修正之处】---
  // 之前我使用了 '{}', 导致了模板推导失败。
  // 现在我显式地创建了正确类型的空vector，以修复此问题。
  auto node_a = graph.make_node(Matrix({{1, 2}, {3, 4}}),
                                std::vector<std::weak_ptr<Node>>{}, "A");
  auto node_b = graph.make_node(Matrix({{10, 20}, {30, 40}}),
                                std::vector<std::weak_ptr<Node>>{}, "B");
  auto node_c = graph.make_node(Matrix({{5}, {6}}),
                                std::vector<std::weak_ptr<Node>>{}, "C");

  {
    // 2.1 加法运算测试
    std::string test_name = "Op 'add': Forward pass value";
    bool passed = false;
    try {
      auto node_d = add(graph, node_a, node_b);
      Matrix expected_d_value({{11, 22}, {33, 44}});
      passed = compare_matrices(node_d->value, expected_d_value);

      // 2.2 计算图结构测试 (add)
      if (passed) {
        test_name = "Op 'add': Graph structure (parents)";
        passed = (node_d->parent.size() == 2);
        if (passed) {
          auto p1 = node_d->parent[0].lock();
          auto p2 = node_d->parent[1].lock();
          // 检查是否正确指向 a 和 b
          passed =
              (p1 == node_a && p2 == node_b) || (p1 == node_b && p2 == node_a);
        }
      }
    } catch (const std::exception& e) {
      passed = false;
      std::cerr << e.what() << '\n';
    }
    print_test_result(test_name, passed);
    if (!passed) all_tests_passed = false;

    // 2.3 乘法运算测试
    test_name = "Op 'mul': Forward pass value";
    passed = false;
    try {
      auto node_e = mul(graph, node_a, node_c);  // (2x2) * (2x1) -> (2x1)
      // 1*5 + 2*6 = 17
      // 3*5 + 4*6 = 15 + 24 = 39
      Matrix expected_e_value({{17}, {39}});
      passed = compare_matrices(node_e->value, expected_e_value);

      // 2.4 计算图结构测试 (mul)
      if (passed) {
        test_name = "Op 'mul': Graph structure (parents)";
        passed = (node_e->parent.size() == 2 && node_e->name == "mul");
        if (passed) {
          auto p1 = node_e->parent[0].lock();
          auto p2 = node_e->parent[1].lock();
          passed =
              (p1 == node_a && p2 == node_c) || (p1 == node_c && p2 == node_a);
        }
      }
    } catch (const std::exception& e) {
      passed = false;
      std::cerr << e.what() << '\n';
    }
    print_test_result(test_name, passed);
    if (!passed) all_tests_passed = false;
  }

  // --- 3. _backward 闭包功能测试 ---
  std::cout << "\n--- Running _backward Lambda Test Suite ---\n";
  {
    // 3.1 测试 add 的 _backward
    std::string test_name = "Op 'add': _backward lambda correctness";
    bool passed = false;
    try {
      auto d_node = add(graph, node_a, node_b);
      // 设定输出节点的梯度
      d_node->grad = Matrix::ones(2, 2);

      // 调用反向传播闭包
      d_node->_backward();

      // 验证父节点梯度是否正确更新
      // a->grad = a->grad + d->grad, 因为 a->grad 初始为0, 所以 a->grad =
      // d->grad
      passed = compare_matrices(node_a->grad, d_node->grad) &&
               compare_matrices(node_b->grad, d_node->grad);
    } catch (const std::exception& e) {
      passed = false;
      std::cerr << e.what() << '\n';
    }
    print_test_result(test_name, passed);
    if (!passed) all_tests_passed = false;

    // 3.2 测试 mul 的 _backward
    test_name = "Op 'mul': _backward lambda correctness";
    passed = false;
    try {
      // 清零之前的梯度，以免受 add 测试影响
      node_a->grad = Matrix::zeros(2, 2);
      node_c->grad = Matrix::zeros(2, 1);

      auto e_node = mul(graph, node_a, node_c);
      e_node->grad = Matrix::ones(2, 1);  // 设定输出梯度

      // 调用反向传播闭包
      e_node->_backward();

      // 验证父节点梯度
      // a->grad = a->grad + e->grad * c->value.T()
      // (2x1) * (1x2) -> (2x2)
      // [[1],[1]] * [[5, 6]] = [[5, 6], [5, 6]]
      Matrix expected_a_grad = e_node->grad * node_c->value.transpose();

      // b->grad = b->grad + a->value.T() * e->grad
      // (2x2) * (2x1) -> (2x1)
      // [[1, 3], [2, 4]] * [[1], [1]] = [[1*1+3*1], [2*1+4*1]] = [[4], [6]]
      Matrix expected_c_grad = node_a->value.transpose() * e_node->grad;

      passed = compare_matrices(node_a->grad, expected_a_grad) &&
               compare_matrices(node_c->grad, expected_c_grad);

    } catch (const std::exception& e) {
      passed = false;
      std::cerr << e.what() << '\n';
    }
    print_test_result(test_name, passed);
    if (!passed) all_tests_passed = false;
  }

  std::cout << "\n--- Test Suite Finished ---\n";
  if (all_tests_passed) {
    std::cout << "All tests passed successfully!\n";
    return 0;
  } else {
    std::cerr << "Some tests failed. Please review the output.\n";
    return 1;
  }
}
