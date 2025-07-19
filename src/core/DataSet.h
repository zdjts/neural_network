#ifndef DATASET_H
#define DATASET_H
#include <utility>

#include "Matrix.h"
class Dataset {
 public:
  virtual ~Dataset() = default;
  virtual int size() const = 0;
  virtual std::pair<Matrix, Matrix> get_item(int index) const = 0;
};

#endif  // !DATASET_H
