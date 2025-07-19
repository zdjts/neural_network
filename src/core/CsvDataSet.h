#ifndef CSVDATASET_H
#define CSVDATASET_H
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "DataSet.h"
#include "Matrix.h"
#include "Node.h"
class CsvDataSet : public Dataset {
 private:
  Matrix features_;
  Matrix labels_;

 public:
  int size() const;

  CsvDataSet(std::pair<Matrix, Matrix> p);
  std::pair<Matrix, Matrix> load_from_file(const std::string& path,
                                           bool normalize);
  void load_from_csv(const std::string& filepath, bool normalize);
  CsvDataSet(const std::string& filepath, bool normalize = true);
  std::pair<Matrix, Matrix> get_item(int index) const;
  const Matrix& get_features() const;
  const Matrix& get_labels() const;
};

void load_parameters(
    const std::string& filepath,
    std::map<std::string, std::shared_ptr<Node>>& parameters_map);

/*void save_parameters(const std::string& filepath, const
 * std::vector<std::shared_ptr<Node>>& parameters);*/
void save_parameters(const std::string& filepath,
                     const std::vector<std::shared_ptr<Node>>& parameters);
#endif  // CSVDATASET_H
