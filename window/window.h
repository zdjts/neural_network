// ===================================================================
// mainwindow.h
// 主窗口组件的头文件
//
// 职责:
// 1. 声明 MainWindow 类。
// 2. 声明所有UI控件的成员变量。
// 3. 声明响应用户操作的槽函数。
// ===================================================================
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// 前向声明，减少头文件依赖
class DrawingCanvas;
class QPushButton;
class QSlider;
class QLabel;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  // **重要提示**: 为了让 MainWindow 能够获取画布图像,
  // 你需要在 DrawingCanvas 类中添加一个公共的 `getImage` 方法。
  // 在 drawingcanvas.h 的 public 部分添加:
  // const QImage &getImage() const;
  //
  // 在 drawingcanvas.cpp 中添加实现:
  // const QImage &DrawingCanvas::getImage() const { return image; }

 private slots:
  void recognizeImage();                // "识别"按钮的核心槽函数
  void updatePenWidthLabel(int width);  // 更新笔宽显示标签

 private:
  // UI 控件
  DrawingCanvas *canvas;
  QPushButton *recognizeButton;
  QPushButton *clearButton;
  QSlider *penWidthSlider;
  QLabel *penWidthLabel;
  QLabel *resultLabel;  // 新增：用于显示识别结果的标签
};

#endif  // MAINWINDOW_H
