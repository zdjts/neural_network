#include <cmath>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

// 包含Matrix类的定义
#include "../src/core/Matrix.h"

void print_test_result(const std::string& test_name, bool passed) {
  // 使用ANSI转义序列为失败的测试添加红色高亮
  std::cout << "[" << (passed ? "PASS" : "\x1B[31mFAIL\x1B[0m") << "] "
            << test_name << std::endl;
}

bool are_nearly_equal(float a, float b, float epsilon = 1e-6f) {
  return std::abs(a - b) < epsilon;
}

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

int main() {
  bool all_tests_passed = true;

  auto run_test = [&](const std::string& name, bool result) {
    print_test_result(name, result);
    if (!result) all_tests_passed = false;
  };

  std::cout << "--- Running Matrix Class Test Suite ---\n";

  // --- 2.1 构造函数和静态方法测试 ---
  std::cout << "\n--- Section 2.1: Constructors & Static Methods ---\n";
  {
    // 默认构造
    Matrix m_zeros(2, 3);
    run_test("Constructor: Zeros",
             compare_matrices(m_zeros, Matrix(2, 3, 0.0f)));

    // 特定值填充构造
    Matrix m_fives(3, 2, 5.0f);
    run_test("Constructor: Fill with value",
             compare_matrices(m_fives, Matrix(3, 2, 5.0f)));

    // 从vector构造
    Matrix m_from_vec({{1, 2}, {3, 4}});
    run_test("Constructor: From vector",
             m_from_vec(0, 0) == 1 && m_from_vec(1, 1) == 4);

    // 静态方法
    run_test("Static: zeros()",
             compare_matrices(Matrix::zeros(2, 2), Matrix(2, 2, 0.0f)));
    run_test("Static: ones()",
             compare_matrices(Matrix::ones(2, 2), Matrix(2, 2, 1.0f)));
    run_test("Static: identity()",
             compare_matrices(Matrix::identity(2), Matrix({{1, 0}, {0, 1}})));
  }

  // --- 2.2 核心功能测试 ---
  std::cout << "\n--- Section 2.2: Core Functionality ---\n";
  {
    Matrix m({{1, 2, 3}, {4, 5, 6}});
    Matrix m_T_expected({{1, 4}, {2, 5}, {3, 6}});
    run_test("Function: transpose()",
             compare_matrices(m.transpose(), m_T_expected));

    Matrix ma({{1, 2}, {3, 4}});
    Matrix mb({{5, 6}, {7, 8}});
    run_test("Function: element_mul()",
             compare_matrices(ma.element_mul(mb), Matrix({{5, 12}, {21, 32}})));

    Matrix m_sum_in({{1, 2, 3}, {4, 5, 6}});
    run_test("Function: sum()",
             compare_matrices(m_sum_in.sum(), Matrix({{21}})));
    run_test("Function: sum_rows()",
             compare_matrices(m_sum_in.sum_rows(), Matrix({{5, 7, 9}})));

    Matrix m_row = m_sum_in.get_row(1);
    run_test("Function: get_row()",
             compare_matrices(m_row, Matrix({{4, 5, 6}})));
  }

  // --- 2.3 运算符重载测试 ---
  std::cout << "\n--- Section 2.3: Operator Overloads ---\n";
  {
    Matrix a({{1, 2}, {3, 4}});
    Matrix b({{10, 20}, {30, 40}});

    // 加法
    Matrix add_expected({{11, 22}, {33, 44}});
    run_test("Operator: + (Matrix + Matrix)",
             compare_matrices(a + b, add_expected));

    // 减法
    Matrix sub_expected({{-9, -18}, {-27, -36}});
    run_test("Operator: - (Matrix - Matrix)",
             compare_matrices(a - b, sub_expected));

    // 矩阵乘法
    Matrix mul_expected({{70, 100}, {150, 220}});
    run_test("Operator: * (Matrix * Matrix)",
             compare_matrices(a * b, mul_expected));

    // 标量乘法
    Matrix scalar_mul_expected({{2, 4}, {6, 8}});
    run_test("Operator: * (Matrix * float)",
             compare_matrices(a * 2.0f, scalar_mul_expected));
  }

  // --- 2.4 异常处理测试 ---
  std::cout << "\n--- Section 2.4: Exception Handling ---\n";
  {
    Matrix m2x2(2, 2);
    Matrix m3x3(3, 3);

    try {
      Matrix res = m2x2 + m3x3;
      run_test("Exception: Add with dimension mismatch", false);
    } catch (const std::invalid_argument&) {
      run_test("Exception: Add with dimension mismatch", true);
    }

    try {
      Matrix res = m2x2 * m3x3;
      run_test("Exception: Multiply with dimension mismatch", false);
    } catch (const std::invalid_argument&) {
      run_test("Exception: Multiply with dimension mismatch", true);
    }

    try {
      m2x2(5, 5) = 1.0f;
      run_test("Exception: Out-of-bounds access", false);
    } catch (const std::out_of_range&) {
      run_test("Exception: Out-of-bounds access", true);
    }
  }

  // --- 最终测试总结 ---
  std::cout << "\n--- Test Suite Finished ---\n";
  if (all_tests_passed) {
    std::cout << "\x1B[32mAll tests passed successfully!\x1B[0m\n";
    return 0;  // 成功
  } else {
    std::cerr << "\x1B[31mSome tests failed. Please review the output "
                 "above.\x1B[0m\n";
    return 1;  // 失败
  }
}
