#include "window.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>
#include <stdexcept>
#include "drawingcanvas.h"


#include "CsvDataSet.h"  
#include "ops.h"


int get_predicted_class(const Matrix &prediction_row) {
  int max_idx = 0;
  for (int i = 1; i < prediction_row.get_cols(); ++i) {
    if (prediction_row(0, i) > prediction_row(0, max_idx)) {
      max_idx = i;
    }
  }
  return max_idx;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  
  canvas = new DrawingCanvas(this);
  canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  
  recognizeButton = new QPushButton("识别", this);
  clearButton = new QPushButton("清除", this);

  
  penWidthSlider = new QSlider(Qt::Horizontal, this);
  penWidthSlider->setRange(10,
                           50);  
  penWidthSlider->setValue(25);

  QLabel *sliderStaticLabel = new QLabel("画笔粗细:", this);
  penWidthLabel = new QLabel(QString::number(penWidthSlider->value()), this);

  
  resultLabel = new QLabel("识别结果: --", this);
  resultLabel->setMinimumWidth(150);

  
  QFont font = resultLabel->font();  
  font.setPointSize(48);             
  resultLabel->setFont(font);        
  resultLabel->setAlignment(Qt::AlignCenter);  
  

  
  QHBoxLayout *controlLayout = new QHBoxLayout;
  controlLayout->addWidget(sliderStaticLabel);
  controlLayout->addWidget(penWidthSlider);
  controlLayout->addWidget(penWidthLabel);
  controlLayout->addStretch();
  controlLayout->addWidget(resultLabel);
  controlLayout->addSpacing(20);
  controlLayout->addWidget(recognizeButton);
  controlLayout->addWidget(clearButton);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(canvas);
  mainLayout->addLayout(controlLayout);

  QWidget *centralWidget = new QWidget(this);
  centralWidget->setLayout(mainLayout);
  setCentralWidget(centralWidget);

  
  connect(clearButton, &QPushButton::clicked, canvas,
          &DrawingCanvas::clearCanvas);
  connect(recognizeButton, &QPushButton::clicked, this,
          &MainWindow::recognizeImage);
  connect(penWidthSlider, &QSlider::valueChanged, canvas,
          &DrawingCanvas::setPenWidth);
  connect(penWidthSlider, &QSlider::valueChanged, this,
          &MainWindow::updatePenWidthLabel);

  
  setWindowTitle("手写数字识别");
  resize(1200, 1200);

  
  try {
    initializeNetwork();
  } catch (const std::exception &e) {
    QMessageBox::critical(this, "错误",
                          QString("模型加载失败: %1").arg(e.what()));
    recognizeButton->setEnabled(false);  
  }
}

MainWindow::~MainWindow() {}

void MainWindow::updatePenWidthLabel(int width) {
  penWidthLabel->setText(QString::number(width));
}


void MainWindow::initializeNetwork() {
  graph = std::make_unique<Graph>();
  int input_size = 784;
  int hidden_size = 128;  
  int output_size = 10;

  
  W1 = graph->make_node(Matrix(input_size, hidden_size), "W1");
  b1 = graph->make_node(Matrix(1, hidden_size), "b1");
  W2 = graph->make_node(Matrix(hidden_size, output_size), "W2");
  b2 = graph->make_node(Matrix(1, output_size), "b2");

  
  std::map<std::string, std::shared_ptr<Node>> params = {
      {"W1", W1}, {"b1", b1}, {"W2", W2}, {"b2", b2}};

  
  
  
  QString appDir = QApplication::applicationDirPath();
  QString modelPath =
      appDir + "/../models/MNIST_1_sigmoid.model";  

  load_parameters(modelPath.toStdString(), params);
}


int MainWindow::predict(const Matrix &imageMatrix) {
  
  auto x = graph->make_node(imageMatrix, "X");

  
  
  auto z1 = add_with_broadcast(*graph, mul(*graph, x, W1), b1);
  auto a1 = sigmoid(*graph, z1);  
  auto z2 = add_with_broadcast(*graph, mul(*graph, a1, W2), b2);

  
  return get_predicted_class(z2->value);
}


void MainWindow::recognizeImage() {
  const QImage &originalImage = canvas->getImage();
  if (originalImage.isNull()) {
    QMessageBox::warning(this, "警告", "画布为空，无法识别。");
    return;
  }

  

  
  int minX = originalImage.width(), minY = originalImage.height(), maxX = 0,
      maxY = 0;
  bool foundPixel = false;
  for (int y = 0; y < originalImage.height(); ++y) {
    for (int x = 0; x < originalImage.width(); ++x) {
      
      if (qGray(originalImage.pixel(x, y)) < 128) {
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
        foundPixel = true;
      }
    }
  }

  if (!foundPixel) {
    QMessageBox::warning(this, "警告", "画布为空或笔迹太浅，无法识别。");
    return;
  }

  
  int padding = penWidthSlider->value() / 2;  
  minX = qMax(0, minX - padding);
  minY = qMax(0, minY - padding);
  maxX = qMin(originalImage.width() - 1, maxX + padding);
  maxY = qMin(originalImage.height() - 1, maxY + padding);

  QRect boundingBox(minX, minY, maxX - minX + 1, maxY - minY + 1);
  QImage croppedImage = originalImage.copy(boundingBox);

  
  int size = qMax(croppedImage.width(), croppedImage.height());
  QImage squareImage(size, size, QImage::Format_ARGB32);
  squareImage.fill(Qt::white);  
  QPainter painter(&squareImage);
  int xOffset = (size - croppedImage.width()) / 2;
  int yOffset = (size - croppedImage.height()) / 2;
  painter.drawImage(xOffset, yOffset, croppedImage);
  painter.end();

  
  QImage finalImage =
      squareImage.scaled(28, 28, Qt::KeepAspectRatio, Qt::SmoothTransformation);

  
  Matrix imageMatrix(1, 784);
  for (int y = 0; y < 28; ++y) {
    for (int x = 0; x < 28; ++x) {
      
      int gray = qGray(finalImage.pixel(x, y));
      
      imageMatrix(0, y * 28 + x) = (255.0f - gray) / 255.0f;
    }
  }

  try {
    int predictionResult = predict(imageMatrix);
    resultLabel->setText(QString("识别结果: %1").arg(predictionResult));
  } catch (const std::exception &e) {
    QMessageBox::critical(this, "预测错误",
                          QString("执行神经网络预测时出错: %1").arg(e.what()));
  }
}
