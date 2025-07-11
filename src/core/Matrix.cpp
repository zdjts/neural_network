#include "Matrix.h"

#include <iomanip>
#include <iostream>
#include <stdexcept>
void Matrix::clear_grad() { std::fill(data_.begin(), data_.end(), 0.0f); };
void Matrix::clear() { std::fill(data_.begin(), data_.end(), 0.0f); };
Matrix::Matrix(int r, int c) : rows_(r), cols_(c), data_(r * c, 0.0f) {}
Matrix::Matrix(const std::vector<std::vector<float>>& data)
    : rows_(data.size()),
      cols_(data.empty() ? 0 : data[0].size()),
      data_(rows_ * cols_, 0.0f) {
  int num = 0;
  for (int i = 0; i < rows_; ++i) {
    if (data[i].size() != cols_) {
      throw std::invalid_argument("Input data is not a regular matrix.");
    }
    for (int j = 0; j < cols_; ++j) {
      data_[num] = data[i][j];
      num++;
    }
  }
}
Matrix::Matrix(int r, int c, float num)
    : rows_(r), cols_(c), data_(r * c, num) {}

Matrix Matrix::transpose() {
  Matrix n_matrix(cols_, rows_);

  for (int i = 0; i < cols_; ++i) {
    for (int j = 0; j < rows_; ++j) {
      n_matrix(i, j) = (*this)(j, i);
    }
  }
  return n_matrix;
}

Matrix Matrix::zeros(int r, int c) {
  Matrix m(r, c);
  return m;
}
Matrix Matrix::ones(int r, int c) {
  Matrix m(r, c, 1.0);
  return m;
}

Matrix Matrix::identity(int size) {
  Matrix m(size, size);
  for (int i = 0; i < size; ++i) {
    m(i, i) = 1;
  }
  return m;
}

float& Matrix::operator()(int r, int c) {
  if (r >= rows_ || c >= cols_) {
    throw std::out_of_range("重载() 访问超出边界\n");
  }
  return data_[r * cols_ + c];
}

const float& Matrix::operator()(int r, int c) const {
  if (r >= rows_ || c >= cols_) {
    throw std::out_of_range("重载() 访问超出边界\n");
  }
  return data_[r * cols_ + c];
}

Matrix Matrix::operator+(const Matrix& other) const {
  if (other.get_rows() != rows_ || other.get_cols() != cols_) {
    throw std::invalid_argument(
        "Matrix addition: Dimensions mismatch. "
        "Left matrix is " +
        std::to_string(rows_) + "x" + std::to_string(cols_) +
        ", Right matrix is " + std::to_string(other.get_rows()) + "x" +
        std::to_string(other.get_cols()) + ".");
  }
  Matrix m(rows_, cols_);
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      m(i, j) = other(i, j) + (*this)(i, j);
    }
  }
  return m;
}

Matrix Matrix::sum() const {
  Matrix m(1, 1);
  for (auto i : data_) {
    m(0, 0) += i;
  }
  return m;
}

Matrix Matrix::operator-(const Matrix& other) const {
  if (other.get_rows() != rows_ || other.get_cols() != cols_) {
    throw std::invalid_argument(
        "Matrix subtraction: Dimensions mismatch. "
        "Left matrix is " +
        std::to_string(rows_) + "x" + std::to_string(cols_) +
        ", Right matrix is " + std::to_string(other.get_rows()) + "x" +
        std::to_string(other.get_cols()) + ".");
  }
  Matrix m(rows_, cols_);
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      m(i, j) = (*this)(i, j) - other(i, j);
    }
  }
  return m;
}

Matrix Matrix::operator*(const Matrix& other) const {
  if (cols_ != other.get_rows()) {
    throw std::invalid_argument(
        "Matrix multiplication: Dimensions mismatch. "
        "Left matrix is " +
        std::to_string(rows_) + "x" + std::to_string(cols_) +
        ", Right matrix is " + std::to_string(other.get_rows()) + "x" +
        std::to_string(other.get_cols()) + ".");
  }
  int c = other.get_cols();
  Matrix m(rows_, c);
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < c; ++j) {
      float sum = 0.0f;
      for (int z = 0; z < cols_; ++z) {
        sum += (*this)(i, z) * other(z, j);
      }
      m(i, j) = sum;
    }
  }
  return m;
}

void Matrix::print() const {
  // 首先打印矩阵的维度信息，让输出更清晰
  std::cout << "Matrix (" << rows_ << "x" << cols_ << "):" << std::endl;

  // 设置输出格式，让浮点数以固定小数位数显示
  // std::fixed: 使用定点表示法
  // std::setprecision(4): 设置小数点后保留4位
  std::cout << std::fixed << std::setprecision(4);

  for (int i = 0; i < rows_; ++i) {
    std::cout << "[ ";
    for (int j = 0; j < cols_; ++j) {
      // std::setw(10): 设置每个数字的输出宽度为10个字符，有助于对齐
      // (*this)(i, j): 使用我们重载的const版本operator()来安全地访问元素
      std::cout << std::setw(10) << (*this)(i, j) << " ";
    }
    std::cout << "]" << std::endl;
  }
  // （可选）恢复cout的默认格式
  std::cout.copyfmt(std::ios(nullptr));
}

Matrix Matrix::element_mul(const Matrix& other) const {
  if (other.get_cols() != cols_ || other.get_rows() != rows_) {
    throw std::invalid_argument("调用element_mul 参数不匹配");
  }
  Matrix m(rows_, cols_);
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      m(i, j) = other(i, j) * (*this)(i, j);
    }
  }
  return m;
}

Matrix Matrix::sum_rows() const {
  Matrix result(1, cols_);  // 结果是一个 1xN 的行向量
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      result(0, j) += (*this)(i, j);
    }
  }
  return result;
}

Matrix Matrix::operator*(float scalar) const {
  Matrix result(rows_, cols_);
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      result(i, j) = (*this)(i, j) * scalar;
    }
  }
  return result;
}

Matrix Matrix::get_row(int row_index) const {
  // 1. 安全性检查：确保行索引没有越界
  if (row_index < 0 || row_index >= rows_) {
    throw std::out_of_range("行索引越界 in Matrix::get_row");
  }

  // 2. 创建一个新的 1 x N 的行矩阵
  Matrix row_matrix(1, cols_);

  // 3. 从当前矩阵中拷贝指定行的数据
  for (int j = 0; j < cols_; ++j) {
    row_matrix(0, j) = (*this)(row_index, j);
  }

  // 4. 返回这个新创建的行矩阵
  return row_matrix;
}

std::vector<float>& Matrix::get_data() { return data_; }

float* Matrix::get_data_ptr() { return data_.data(); }
const float* Matrix::get_data_ptr() const { return data_.data(); }
