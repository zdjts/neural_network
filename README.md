## 简单神将网络的实现
### 简介
NWAFU 2023级计算机科学与技术数据结构实习题目。
### 项目目标
使用c++标准库实现简单神经网络并通过手写数字识别验证神经网络可行性。
### 使用方法
```shell
mkdir build
cd ./build
cmake ..
make
./window
```

### 文件组成
  - `app` 图形界面相关代码。
  - `src` 神经网络框架以及数据读取相关文件。
  - `ops` 函数运算操作相关。
  - `data` 训练数据。
  - `models`训练模型。
  - `test` 模块测试相关。
  - `traning` 训练代码。
### 相关环境。
需要用户含有cmake、makefile、以及QT基础库。
QT具体需要`qtbase5-dev qt5-qmake`
为了便于提交，并未包含手写数字识别数据集可通过官网下载,或使用命令。
```shell
git clone https://github.com/zdjts/neural_network.git
``` 
拉取包含数据集的全部代码。
