#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <istream>
#include <ostream>
#include <random>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <type_traits>
#include <utility>
#include <vector>

namespace LinAlg {
template <typename T> class Matrix {
public:
  using element_type = T;
  Matrix();
  Matrix(const uint64_t &rows, const uint64_t &cols);
  void setArr(const std::vector<std::vector<T>> &arr);
  std::vector<std::vector<T>> getArr() const;
  Matrix operator+(const Matrix &M) const;
  Matrix operator-(const Matrix &M) const;
  void operator=(const std::vector<std::vector<T>> &arr);
  bool operator==(const Matrix &M) const;
  template <typename P>
  friend std::ostream &operator<<(std::ostream &output, const Matrix<P> &M);
  template <typename P>
  friend std::istream &operator>>(std::istream &input, Matrix<P> &M);
  std::pair<T, T> shape() const;
  Matrix matmul(const Matrix<T> &M) const;
  Matrix scalarmul(const T &scalar) const;
  Matrix subMatrix(const uint64_t &row_to_be_deleted,
                   const uint64_t &col_to_be_deleted) const;
  T determinant(const std::string &algorithm) const;
  Matrix transpose() const;
  T trace() const;
  double frobNorm() const;
  Matrix<double> inv() const;
  template <typename P> operator P(); // type conversion using static_cast
  friend Matrix<double> randn(const std::pair<uint64_t, uint64_t> &shape,
                              const double &mean, const double &var);
  Matrix<double> rref() const;
  std::pair<Matrix<double>, Matrix<double>> lu() const;
  bool isOrthogonal() const; // checks if the matrix is orthogonal

private:
  uint64_t rows_, cols_;
  std::vector<std::vector<T>> arr_;
  T determinantLaplace() const;
};

template <typename T> Matrix<T>::Matrix() : rows_(0), cols_(0) { ; }

template <typename T>
Matrix<T>::Matrix(const uint64_t &rows, const uint64_t &cols)
    : rows_(rows), cols_(cols) {
  for (uint64_t i = 0; i < rows; i++) {
    std::vector<T> arr;
    for (uint64_t j = 0; j < cols; j++) {
      arr.push_back((T)0);
    }
    this->arr_.push_back(arr);
  }
}

template <typename T>
void Matrix<T>::setArr(const std::vector<std::vector<T>> &arr) {
  this->arr_ = arr;
  return;
}

template <typename T> std::vector<std::vector<T>> Matrix<T>::getArr() const {
  return this->arr_;
}

template <typename T> Matrix<T> Matrix<T>::operator+(const Matrix &M) const {
  if (M.rows_ != this->rows_ || M.cols_ != this->cols_)
    throw std::invalid_argument("Matrices must be of the same shape\n");
  Matrix<T> sum(this->rows_, this->cols_);
  for (uint64_t i = 0; i < this->rows_; i++) {
    for (uint64_t j = 0; j < this->cols_; j++) {
      sum.arr_[i][j] = this->arr_[i][j] + M.arr_[i][j];
    }
  }
  return sum;
}

template <typename T> Matrix<T> Matrix<T>::operator-(const Matrix<T> &M) const {
  if (M.rows_ != this->rows_ || M.cols_ != this->cols_)
    throw std::invalid_argument("Matrices must be of the same shape");
  Matrix<T> minus(this->rows_, this->cols_);
  for (uint64_t i = 0; i < this->rows_; i++) {
    for (uint64_t j = 0; j < this->cols_; j++) {
      minus.arr_[i][j] = this->arr_[i][j] - M.arr_[i][j];
    }
  }
  return minus;
}

template <typename T>
void Matrix<T>::operator=(const std::vector<std::vector<T>> &arr) {
  this->arr_ = arr;
  return;
}

template <typename T> bool Matrix<T>::operator==(const Matrix<T> &M) const {
  return (this->arr_ == M.arr_);
}

template <typename T>
std::ostream &operator<<(std::ostream &output, const Matrix<T> &M) {
  for (uint64_t i = 0; i < M.rows_; i++) {
    for (uint64_t j = 0; j < M.cols_; j++) {
      output << M.arr_[i][j] << " ";
    }
    output << "\n";
  }
  return output;
}

template <typename T>
std::istream &operator>>(std::istream &input, Matrix<T> &M) {
  for (uint64_t i = 0; i < M.rows_; i++) {
    for (uint64_t j = 0; j < M.cols_; j++) {
      input >> M.arr_[i][j];
    }
  }
  return input;
}

template <typename T> Matrix<T> Matrix<T>::scalarmul(const T &scalar) const {
  Matrix<T> M(this->rows_, this->cols_);
  for (uint64_t i = 0; i < this->rows_; i++) {
    for (uint64_t j = 0; j < this->cols_; j++) {
      M.arr_[i][j] = this->arr_[i][j] * scalar;
    }
  }
  return M;
}

template <typename T> std::pair<T, T> Matrix<T>::shape() const {
  return std::make_pair(this->rows_, this->cols_);
}

template <typename T> Matrix<T> Matrix<T>::matmul(const Matrix<T> &M) const {
  if (this->cols_ != M.rows_)
    throw std::invalid_argument("shape mismatch\n");
  Matrix<T> mult(this->rows_, M.cols_);
  for (uint64_t i = 0; i < this->rows_; i++) {
    for (uint64_t j = 0; j < this->cols_; j++) {
      T element = 0;
      for (uint64_t k = 0; k < this->rows_; k++)
        element += this->arr_[i][k] * M.arr_[k][j];
      mult.arr_[i][j] = element;
    }
  }
  return mult;
}

template <typename T>
Matrix<T> Matrix<T>::subMatrix(const uint64_t &row_to_be_deleted,
                               const uint64_t &col_to_be_deleted) const {
  Matrix<T> submatrix(this->rows_ - 1, this->cols_ - 1);
  for (uint64_t i = 0; i < this->rows_; i++) {
    for (uint64_t j = 0; j < this->cols_; j++) {
      if (i < row_to_be_deleted && j < col_to_be_deleted)
        submatrix.arr_[i][j] = this->arr_[i][j];
      else {
        if (i < row_to_be_deleted && j > col_to_be_deleted) {
          submatrix.arr_[i][j - 1] = this->arr_[i][j];
        } else if (i > row_to_be_deleted && j < col_to_be_deleted) {
          submatrix.arr_[i - 1][j] = this->arr_[i][j];
        } else if (i > row_to_be_deleted && j > col_to_be_deleted) {
          submatrix.arr_[i - 1][j - 1] = this->arr_[i][j];
        }
      }
    }
  }
  return submatrix;
}

template <typename T> T Matrix<T>::determinantLaplace() const {
  if (this->rows_ != this->cols_)
    throw std::invalid_argument(
        "Determinant is only defined on square matrices\n");
  if (this->rows_ == 1)
    return this->arr_[0][0];
  if (this->rows_ == 2) {
    return (this->arr_[1][1] * this->arr_[0][0] -
            this->arr_[1][0] * this->arr_[0][1]);
  }
  T det = 0;
  for (uint64_t j = 1; j <= this->cols_; j++) {
    if ((j + 1) % 2 == 0) /* i + j  is even */ {
      det += -this->arr_[0][j - 1] *
             (this->subMatrix(0, j - 1).determinantLaplace());
    } else {
      det += this->arr_[0][j - 1] *
             (this->subMatrix(0, j - 1).determinantLaplace());
    }
  }
  return det;
}

template <typename T>
T Matrix<T>::determinant(const std::string &algorithm) const {
  if (algorithm == "laplace") {
    return this->determinantLaplace();
  }
  return this->determinantLaplace();
}

template <typename T> Matrix<T> Matrix<T>::transpose() const {
  uint64_t rows = this->cols_;
  uint64_t cols = this->rows_;
  Matrix<T> m(rows, cols);
  for (uint64_t i = 0; i < rows; i++) {
    for (uint64_t j = 0; j < cols; j++) {
      m.arr_[i][j] = this->arr_[j][i];
    }
  }
  return m;
}

template <typename T> T Matrix<T>::trace() const {
  if (this->rows_ != this->cols_)
    throw std::invalid_argument("trace is only defined for square matrices\n");
  T tr = 0;
  for (uint64_t i = 0; i < this->rows_; i++)
    tr += this->arr_[i][i];
  return tr;
}

template <typename T> double Matrix<T>::frobNorm() const {
  if (this->rows_ != this->cols_)
    throw std::invalid_argument("Norm is defined only for squared matrices\n");
  double norm = 0;
  for (uint64_t i = 0; i < this->rows_; i++) {
    for (uint64_t j = 0; j < this->cols_; j++) {
      norm += this->arr_[i][j] * this->arr_[i][j];
    }
  }
  return norm;
}

template <typename T> template <typename P> Matrix<T>::operator P() {
  if constexpr (std::is_arithmetic_v<typename P::element_type>) {
    std::vector<std::vector<typename P::element_type>> arr;
    for (uint64_t i = 0; i < this->rows_; i++) {
      std::vector<typename P::element_type> temp;
      for (uint64_t j = 0; j < this->cols_; j++) {
        temp.push_back(typename P::element_type(this->arr_[i][j]));
      }
      arr.push_back(temp);
    }
    Matrix<typename P::element_type> m(this->rows_, this->cols_);
    m.setArr(arr);
    return m;
  }
  throw std::invalid_argument("no defined type conversion\n");
}

template <typename T>
Matrix<double> randn(const std::pair<uint64_t, uint64_t> &shape,
                     const double &mean, const double &var) {
  std::random_device rd;  // seed generator
  std::mt19937 gen(rd()); // Mersenne Twister engine
  std::normal_distribution<> d(mean, std::sqrt(var));
  std::vector<std::vector<double>> arr(shape.first,
                                       std::vector<double>(shape.second));
  for (uint64_t i = 0; i < shape.first; i++) {
    for (uint64_t j = 0; j < shape.second; j++) {
      arr[i][j] = d(gen);
    }
  }
  Matrix<double> random_matrix(shape.first, shape.second);
  random_matrix.setArr(arr);
  return random_matrix;
}

} // namespace LinAlg
