#include <cmath>  // For std::abs for float comparison
#include <iostream>
#include <stdexcept>
#include <string>  // 用于std::to_string
#include <vector>

#include "Matrix.h"  // 包含你的Matrix类定义

// 辅助函数：打印测试结果
void print_test_result(const std::string& test_name, bool passed) {
  std::cout << "[" << (passed ? "PASS" : "FAIL") << "] " << test_name
            << std::endl;
}

// 辅助函数：比较两个浮点数是否近似相等，考虑到浮点精度问题
bool are_nearly_equal(float a, float b, float epsilon = 1e-6f) {
  return std::abs(a - b) < epsilon;
}

// 辅助函数：比较两个矩阵是否近似相等
bool compare_matrices(const Matrix& m1, const Matrix& m2,
                      float epsilon = 1e-6f) {
  if (m1.get_rows() != m2.get_rows() || m1.get_cols() != m2.get_cols()) {
    std::cout << "  Dimension mismatch: (" << m1.get_rows() << "x"
              << m1.get_cols() << ") vs (" << m2.get_rows() << "x"
              << m2.get_cols() << ")\n";
    return false;  // 维度不同
  }
  for (int i = 0; i < m1.get_rows();
       ++i) {  // 注意：这里使用了int循环，因为get_rows/cols返回int
    for (int j = 0; j < m1.get_cols(); ++j) {
      if (!are_nearly_equal(m1(i, j), m2(i, j), epsilon)) {
        std::cout << "  Element mismatch at (" << i << "," << j
                  << "): " << m1(i, j) << " != " << m2(i, j) << "\n";
        return false;  // 元素不匹配
      }
    }
  }
  return true;
}

int main() {
  std::cout << "--- Matrix Class Test Suite ---\n\n";

  // --- 3.1 构造函数测试 ---

  // Test 3.1.1: Matrix(int r, int c) - 默认全零构造
  {
    std::string test_name = "Constructor Matrix(int, int) - default zero fill";
    try {
      Matrix m(2, 3);
      bool passed = (m.get_rows() == 2 && m.get_cols() == 3);
      if (passed) {
        for (int i = 0; i < 2; ++i) {
          for (int j = 0; j < 3; ++j) {
            if (!are_nearly_equal(m(i, j), 0.0f)) {
              passed = false;
              break;
            }
          }
          if (!passed) break;
        }
      }
      print_test_result(test_name, passed);
      if (!passed) {
        std::cout << "  Expected 2x3 all zeros. Actual values:\n";
        m.print();
      }
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Exception: " << e.what() << std::endl;
    }
  }

  // Test 3.1.2: Matrix(int r, int c, float num) - 特定值填充构造
  {
    std::string test_name =
        "Constructor Matrix(int, int, float) - fill with 5.0f";
    try {
      Matrix m(3, 2, 5.0f);
      bool passed = (m.get_rows() == 3 && m.get_cols() == 2);
      if (passed) {
        for (int i = 0; i < 3; ++i) {
          for (int j = 0; j < 2; ++j) {
            if (!are_nearly_equal(m(i, j), 5.0f)) {
              passed = false;
              break;
            }
          }
          if (!passed) break;
        }
      }
      print_test_result(test_name, passed);
      if (!passed) {
        std::cout << "  Expected 3x2 all 5s. Actual values:\n";
        m.print();
      }
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Exception: " << e.what() << std::endl;
    }
  }

  // Test 3.1.3: Matrix(const std::vector<std::vector<float>>& data) -
  // 从二维vector构造
  {
    std::string test_name = "Constructor Matrix(const vector<vector<float>>&)";
    try {
      std::vector<std::vector<float>> input = {
          {1.0f, 2.0f}, {3.0f, 4.0f}, {5.0f, 6.0f}};
      Matrix m(input);
      bool passed = (m.get_rows() == 3 && m.get_cols() == 2);
      if (passed) {
        passed = are_nearly_equal(m(0, 0), 1.0f) &&
                 are_nearly_equal(m(0, 1), 2.0f) &&
                 are_nearly_equal(m(1, 0), 3.0f) &&
                 are_nearly_equal(m(1, 1), 4.0f) &&
                 are_nearly_equal(m(2, 0), 5.0f) &&
                 are_nearly_equal(m(2, 1), 6.0f);
      }
      print_test_result(test_name, passed);
      if (!passed) {
        std::cout
            << "  Expected specific values from input vector. Actual values:\n";
        m.print();
      }
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Exception: " << e.what() << std::endl;
    }
  }

  // Test 3.1.4: Matrix(const std::vector<std::vector<float>>& data) -
  // 不规则输入 (异常测试)
  {
    std::string test_name =
        "Constructor from irregular vector - throws exception";
    try {
      std::vector<std::vector<float>> input = {{1.0f, 2.0f},
                                               {3.0f}};  // Irregular
      Matrix m(input);
      print_test_result(test_name, false);  // Should not reach here
    } catch (const std::invalid_argument& e) {
      print_test_result(test_name, true);  // Correctly caught exception
      // std::cout << "  Caught expected exception: " << e.what() << std::endl;
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Caught unexpected exception: " << e.what() << std::endl;
    }
  }
  // Test 3.1.5: Matrix(const std::vector<std::vector<float>>& data) - 空输入
  {
    std::string test_name = "Constructor from empty vector";
    try {
      std::vector<std::vector<float>> input;
      Matrix m(input);
      bool passed = (m.get_rows() == 0 && m.get_cols() == 0);
      print_test_result(test_name, passed);
      if (!passed) std::cout << "  Expected 0x0 matrix for empty input.\n";
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Exception: " << e.what() << std::endl;
    }
  }

  // --- 3.2 静态工厂方法测试 ---

  // Test 3.2.1: Matrix::zeros(int r, int c)
  {
    std::string test_name = "Static method zeros(int, int)";
    try {
      Matrix m = Matrix::zeros(2, 2);
      bool passed = (m.get_rows() == 2 && m.get_cols() == 2);
      if (passed) {
        for (int i = 0; i < 2; ++i) {
          for (int j = 0; j < 2; ++j) {
            if (!are_nearly_equal(m(i, j), 0.0f)) {
              passed = false;
              break;
            }
          }
          if (!passed) break;
        }
      }
      print_test_result(test_name, passed);
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Exception: " << e.what() << std::endl;
    }
  }

  // Test 3.2.2: Matrix::ones(int r, int c)
  {
    std::string test_name = "Static method ones(int, int)";
    try {
      Matrix m = Matrix::ones(2, 2);
      bool passed = (m.get_rows() == 2 && m.get_cols() == 2);
      if (passed) {
        for (int i = 0; i < 2; ++i) {
          for (int j = 0; j < 2; ++j) {
            if (!are_nearly_equal(m(i, j), 1.0f)) {
              passed = false;
              break;
            }
          }
          if (!passed) break;
        }
      }
      print_test_result(test_name, passed);
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Exception: " << e.what() << std::endl;
    }
  }

  // Test 3.2.3: Matrix::identity(int size)
  {
    std::string test_name = "Static method identity(int)";
    try {
      Matrix m = Matrix::identity(3);
      bool passed = (m.get_rows() == 3 && m.get_cols() == 3);
      if (passed) {
        for (int i = 0; i < 3; ++i) {
          for (int j = 0; j < 3; ++j) {
            if (i == j) {
              if (!are_nearly_equal(m(i, j), 1.0f)) {
                passed = false;
                break;
              }
            } else {
              if (!are_nearly_equal(m(i, j), 0.0f)) {
                passed = false;
                break;
              }
            }
          }
          if (!passed) break;
        }
      }
      print_test_result(test_name, passed);
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Exception: " << e.what() << std::endl;
    }
  }
  // Test 3.2.4: Matrix::identity(0) - 边界情况
  {
    std::string test_name = "Static method identity(0) - 0x0 matrix";
    try {
      Matrix m = Matrix::identity(0);
      bool passed = (m.get_rows() == 0 && m.get_cols() == 0);
      print_test_result(test_name, passed);
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Exception: " << e.what() << std::endl;
    }
  }

  // --- 3.3 元素访问 operator() 测试 ---

  // Test 3.3.1: operator() - 正常访问 (读写)
  {
    std::string test_name = "operator() - read/write access";
    try {
      Matrix m(2, 2);
      m(0, 0) = 10.0f;
      m(1, 1) = 20.0f;
      bool passed =
          are_nearly_equal(m(0, 0), 10.0f) && are_nearly_equal(m(1, 1), 20.0f);
      print_test_result(test_name, passed);
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Exception: " << e.what() << std::endl;
    }
  }

  // Test 3.3.2: operator() - 越界访问 (异常测试)
  {
    std::string test_name = "operator() - out of range access throws exception";
    try {
      Matrix m(2, 2);
      m(2, 0) = 1.0f;                       // This should throw!
      print_test_result(test_name, false);  // Should not reach here
    } catch (const std::out_of_range& e) {
      print_test_result(test_name, true);  // Correctly caught exception
      // std::cout << "  Caught expected exception: " << e.what() << std::endl;
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Caught unexpected exception: " << e.what() << std::endl;
    }
  }

  // Test 3.3.3: operator() - const Matrix 只读访问
  {
    std::string test_name = "operator() - const Matrix read-only access";
    try {
      Matrix temp_m(1, 1, 42.0f);
      const Matrix cm = temp_m;  // Create a const matrix
      bool passed = are_nearly_equal(cm(0, 0), 42.0f);
      print_test_result(test_name, passed);
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Exception: " << e.what() << std::endl;
    }
  }

  // --- 3.4 运算符重载测试 ---

  // Setup for operator tests
  Matrix ma(2, 2, 1.0f);              // All 1s
  Matrix mb(2, 2, 2.0f);              // All 2s
  Matrix mc_expected(2, 2, 3.0f);     // All 3s for addition
  Matrix md_expected(2, 2, -1.0f);    // All -1s for subtraction
  Matrix m_id = Matrix::identity(2);  // 2x2 identity matrix

  // Test 3.4.1: operator+ (Matrix + Matrix) - 正常加法
  {
    std::string test_name = "operator+ (Matrix + Matrix) - normal addition";
    try {
      Matrix sum_m = ma + mb;  // 这里调用了重载的 operator+
      bool passed = compare_matrices(sum_m, mc_expected);
      print_test_result(test_name, passed);
      if (!passed) {
        std::cout << "  Expected:\n";
        mc_expected.print();
        std::cout << "  Actual:\n";
        sum_m.print();
      }
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Exception: " << e.what() << std::endl;
    }
  }

  // Test 3.4.2: operator+ (Matrix + Matrix) - 维度不匹配 (异常测试)
  {
    std::string test_name = "operator+ - dimension mismatch throws exception";
    try {
      Matrix m_diff_dim(2, 3, 1.0f);
      Matrix result = ma + m_diff_dim;  // Should throw
      print_test_result(test_name, false);
    } catch (const std::invalid_argument& e) {
      print_test_result(test_name, true);
      // std::cout << "  Caught expected exception: " << e.what() << std::endl;
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Caught unexpected exception: " << e.what() << std::endl;
    }
  }

  // Test 3.4.3: operator- (Matrix - Matrix) - 正常减法
  {
    std::string test_name = "operator- (Matrix - Matrix) - normal subtraction";
    try {
      Matrix diff_m = ma - mb;  // 1s - 2s = -1s
      bool passed = compare_matrices(diff_m, md_expected);
      print_test_result(test_name, passed);
      if (!passed) {
        std::cout << "  Expected:\n";
        md_expected.print();
        std::cout << "  Actual:\n";
        diff_m.print();
      }
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Exception: " << e.what() << std::endl;
    }
  }

  // Test 3.4.4: operator- (Matrix - Matrix) - 维度不匹配 (异常测试)
  {
    std::string test_name = "operator- - dimension mismatch throws exception";
    try {
      Matrix m_diff_dim(2, 3, 1.0f);
      Matrix result = ma - m_diff_dim;  // Should throw
      print_test_result(test_name, false);
    } catch (const std::invalid_argument& e) {
      print_test_result(test_name, true);
      // std::cout << "  Caught expected exception: " << e.what() << std::endl;
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Caught unexpected exception: " << e.what() << std::endl;
    }
  }

  // Test 3.4.5: operator* (Matrix * Matrix) - 正常乘法
  {
    std::string test_name =
        "operator* (Matrix * Matrix) - normal multiplication (Identity)";
    try {
      Matrix mult_m = ma * m_id;  // Identity matrix should not change ma
      bool passed = compare_matrices(mult_m, ma);
      print_test_result(test_name, passed);
      if (!passed) {
        std::cout << "  Expected:\n";
        ma.print();
        std::cout << "  Actual:\n";
        mult_m.print();
      }
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Exception: " << e.what() << std::endl;
    }
  }
  // Test 3.4.6: operator* (Matrix * Matrix) - 更复杂的乘法
  {
    std::string test_name =
        "operator* (Matrix * Matrix) - complex multiplication";
    try {
      std::vector<std::vector<float>> input1 = {{1.0f, 2.0f},
                                                {3.0f, 4.0f}};  // 2x2
      std::vector<std::vector<float>> input2 = {{5.0f, 6.0f},
                                                {7.0f, 8.0f}};  // 2x2
      Matrix mat1(input1);
      Matrix mat2(input2);

      // Expected result:
      // (1*5 + 2*7) = 5 + 14 = 19
      // (1*6 + 2*8) = 6 + 16 = 22
      // (3*5 + 4*7) = 15 + 28 = 43
      // (3*6 + 4*8) = 18 + 32 = 50
      std::vector<std::vector<float>> expected_output = {{19.0f, 22.0f},
                                                         {43.0f, 50.0f}};
      Matrix mat_expected(expected_output);

      Matrix mat_result = mat1 * mat2;
      bool passed = compare_matrices(mat_result, mat_expected);
      print_test_result(test_name, passed);
      if (!passed) {
        std::cout << "  Expected:\n";
        mat_expected.print();
        std::cout << "  Actual:\n";
        mat_result.print();
      }
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Exception: " << e.what() << std::endl;
    }
  }

  // Test 3.4.7: operator* (Matrix * Matrix) - 维度不匹配 (异常测试)
  {
    std::string test_name = "operator* - dimension mismatch throws exception";
    try {
      Matrix m_diff_dim(3, 2, 1.0f);  // 3x2
      Matrix result =
          ma * m_diff_dim;  // 2x2 * 3x2, inner dimensions (2,3) mismatch
      print_test_result(test_name, false);
    } catch (const std::invalid_argument& e) {
      print_test_result(test_name, true);
      // std::cout << "  Caught expected exception: " << e.what() << std::endl;
    } catch (const std::exception& e) {
      print_test_result(test_name, false);
      std::cout << "  Caught unexpected exception: " << e.what() << std::endl;
    }
  }

  std::cout << "\n--- Test Suite Finished ---\n";

  return 0;
}
