#include "../core/CsvDataSet.cpp"
#include "../src/stb/LoadImage.h"
#include "../src/stb/SaveImage.h"
#include "CsvDataSet.h"
#include "Graph.h"
#include "ops.h"

int main(int argc, char** argv) {
  LoadImage l_image = LoadImage(std::string(argv[1]));

  Matrix x_m = l_image.get_Matrix();

  std::cout << "读取图片为" << argv[1] << std::endl;
  std::cout << "保存读取的图片为" << "new_image.bmp" << std::endl;

  SaveImage::save("new_image.bmp", x_m);

  return 0;
}
