#ifndef MATRIX_H
#define MATRIX_H
#include <vector>
class Matrix {
 private:
  int rows_;
  int cols_;
  std::vector<float> data_;

 public:
  explicit Matrix(int r, int c);
  explicit Matrix(int r, int c, float num);
  explicit Matrix(const std::vector<std::vector<float>>& data);
  [[nodiscard]] int get_rows() const { return rows_; }
  [[nodiscard]] int get_cols() const { return cols_; }

  void clear_grad();
  void clear();
  // 对应位置元素相乘
  Matrix sum() const;
  Matrix element_mul(const Matrix& other) const;
  Matrix transpose();
  static Matrix zeros(int r, int c);
  static Matrix ones(int r, int c);
  static Matrix identity(int size);  // 创建单位矩阵
  Matrix operator+(const Matrix& other) const;
  Matrix operator*(const float num) const;
  Matrix operator*(const Matrix& other) const;
  Matrix operator-(const Matrix& other) const;
  void print() const;
  [[nodiscard]] float& operator()(int r, int c);
  const float& operator()(int r, int c) const;
  Matrix get_row(int row_index) const;
  // 在 Matrix.h 的 public区域添加声明:
  Matrix sum_rows() const;
  float* get_data_ptr();
  const float* get_data_ptr() const;
  std::vector<float>& get_data();
};

#endif  // !MATRIX_H
