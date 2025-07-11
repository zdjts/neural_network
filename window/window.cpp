
// ===================================================================
// mainwindow.cpp
// 主窗口组件的源文件
//
// 职责:
// 1. 创建并布局所有UI控件。
// 2. 实现信号与槽的连接。
// 3. 实现 "识别" 功能的具体逻辑：保存图片 -> 调用外部程序 -> 显示结果。
// ===================================================================
#include "window.h"

#include <QApplication>  // 用于获取可执行文件路径
#include <QDateTime>     // 用于生成唯一文件名
#include <QDebug>        // 用于调试输出
#include <QDir>          // 用于处理目录和路径
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>  // 用于调用外部程序
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>

#include "drawingcanvas.h"  // 需要包含 DrawingCanvas 的定义

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  // 1. 创建核心控件
  canvas = new DrawingCanvas(this);
  canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // 2. 创建按钮
  recognizeButton = new QPushButton("识别", this);
  clearButton = new QPushButton("清除", this);

  // 3. 创建画笔粗细调节控件
  penWidthSlider = new QSlider(Qt::Horizontal, this);
  penWidthSlider->setRange(1, 50);
  penWidthSlider->setValue(5);

  QLabel *sliderStaticLabel = new QLabel("画笔粗细:", this);
  penWidthLabel = new QLabel(QString::number(penWidthSlider->value()), this);

  // 4. 新增: 创建结果显示标签
  resultLabel = new QLabel("识别结果: --", this);
  resultLabel->setMinimumWidth(150);  // 给它一个最小宽度以防文字变化时布局跳动

  // 5. 设置布局
  // 底部控制区域的水平布局
  QHBoxLayout *controlLayout = new QHBoxLayout;
  controlLayout->addWidget(sliderStaticLabel);
  controlLayout->addWidget(penWidthSlider);
  controlLayout->addWidget(penWidthLabel);
  controlLayout->addStretch();
  controlLayout->addWidget(resultLabel);  // 将结果标签放在按钮左边
  controlLayout->addSpacing(20);
  controlLayout->addWidget(recognizeButton);
  controlLayout->addWidget(clearButton);

  // 窗口主体的垂直布局
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(canvas);
  mainLayout->addLayout(controlLayout);

  // 设置中央窗口部件
  QWidget *centralWidget = new QWidget(this);
  centralWidget->setLayout(mainLayout);
  setCentralWidget(centralWidget);

  // 6. 连接信号和槽
  connect(clearButton, &QPushButton::clicked, canvas,
          &DrawingCanvas::clearCanvas);
  connect(recognizeButton, &QPushButton::clicked, this,
          &MainWindow::recognizeImage);
  connect(penWidthSlider, &QSlider::valueChanged, canvas,
          &DrawingCanvas::setPenWidth);
  connect(penWidthSlider, &QSlider::valueChanged, this,
          &MainWindow::updatePenWidthLabel);

  // 7. 设置窗口属性
  setWindowTitle("手写识别");
  resize(600, 600);
}

MainWindow::~MainWindow() {}

void MainWindow::updatePenWidthLabel(int width) {
  penWidthLabel->setText(QString::number(width));
}

// "识别"按钮的槽函数实现
void MainWindow::recognizeImage() {
  // 步骤 1: 定义路径和文件名
  QString appDir = QApplication::applicationDirPath();
  QString executablePath = appDir + "/test_load_image";
  QString tempImageDir = appDir + "/temp_images";

  // 检查并创建临时图片目录
  QDir dir(tempImageDir);
  if (!dir.exists()) {
    if (!dir.mkpath(".")) {
      QMessageBox::critical(this, "错误",
                            "无法创建临时图片目录: " + tempImageDir);
      return;
    }
  }
  QString imagePath =
      tempImageDir +
      QString("/input_%1.png").arg(QDateTime::currentMSecsSinceEpoch());

  // 步骤 2: 保存画布内容为图片文件
  // **注意**: 此处调用了你在 DrawingCanvas 中添加的 `getImage()` 公共方法
  const QImage &imageToSave = canvas->getImage();

  // --- 新增的预处理逻辑 ---
  // 1. 寻找边界框
  int minX = imageToSave.width(), minY = imageToSave.height(), maxX = 0,
      maxY = 0;
  bool foundPixel = false;
  for (int y = 0; y < imageToSave.height(); ++y) {
    for (int x = 0; x < imageToSave.width(); ++x) {
      // 检查像素是否为黑色（笔迹）
      if (qGray(imageToSave.pixel(x, y)) < 255) {
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
        foundPixel = true;
      }
    }
  }

  if (imageToSave.isNull()) {
    QMessageBox::warning(this, "警告", "画布为空，无法识别。");
    return;
  }

  QRect boundingBox(minX, minY, maxX - minX + 1, maxY - minY + 1);
  QImage croppedImage = imageToSave.copy(boundingBox);

  // 3. 将裁剪后的图像调整为正方形（添加白边）
  int size = qMax(croppedImage.width(), croppedImage.height());
  QImage squareImage(size, size, QImage::Format_RGB32);
  squareImage.fill(Qt::white);  // 填充白色背景
  QPainter painter(&squareImage);
  int xOffset = (size - croppedImage.width()) / 2;
  int yOffset = (size - croppedImage.height()) / 2;
  painter.drawImage(xOffset, yOffset, croppedImage);
  painter.end();

  // 保存正方形图片
  if (!squareImage.save(imagePath, "PNG")) {
    QMessageBox::critical(this, "错误", "无法保存图片到: " + imagePath);
    return;
  }

  // 步骤 3: 调用外部识别程序
  QProcess process;
  QStringList arguments;
  arguments << imagePath;

  // 设置进程的工作目录为可执行文件所在目录
  process.setWorkingDirectory(appDir);

  // 启动外部进程
  process.start(executablePath, arguments);

  // 同步等待进程完成，设置超时时间为10秒。
  // 对于耗时长的任务，建议使用信号槽机制实现异步调用，避免界面卡死。
  if (!process.waitForFinished(10000)) {
    QMessageBox::critical(this, "错误",
                          "调用识别程序超时或启动失败。\n请确认 " +
                              executablePath + " 存在且可执行。");
    qWarning() << "Process Error:" << process.errorString();
    return;
  }

  // 检查进程是否正常退出
  if (process.exitStatus() != QProcess::NormalExit) {
    QMessageBox::critical(this, "错误", "识别程序异常退出。");
    qWarning() << "Process Error:" << process.errorString();
    qWarning() << "Standard Output:" << process.readAllStandardOutput();
    qWarning() << "Standard Error:" << process.readAllStandardError();
    return;
  }

  // 步骤 4: 获取进程的退出码 (返回值) 并更新UI
  int predictionResult = process.exitCode();
  resultLabel->setText(QString("识别结果: %1").arg(predictionResult));

  // (可选) 步骤 5: 清理本次生成的临时图片
  // QFile::remove(imagePath);
}
