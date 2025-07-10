// ===================================================================
// drawingcanvas.h
// 绘图画布组件的头文件
//
// 职责:
// 1. 声明 DrawingCanvas 类及其继承关系。
// 2. 声明对外开放的接口 (slots)，如清除画布、设置笔宽。
// 3. 声明需要响应的鼠标和绘图事件。
// 4. 声明内部使用的私有成员变量和辅助函数。
// ===================================================================
#ifndef DRAWINGCANVAS_H
#define DRAWINGCANVAS_H

#include <QImage>
#include <QPoint>
#include <QWidget>

// 前向声明，避免在头文件中包含大型头文件
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;

class DrawingCanvas : public QWidget {
  // Q_OBJECT 宏是所有使用信号和槽的Qt类的必需品
  Q_OBJECT

 public:
  // 构造函数，'explicit' 防止隐式类型转换
  explicit DrawingCanvas(QWidget *parent = nullptr);

  // 公共槽函数 (Public Slots): 这些是外部可以调用的接口
 public slots:
  void clearCanvas();              // 清除整个画布
  void setPenWidth(int newWidth);  // 设置画笔的粗细

  // 新增: 获取当前画布图像
  const QImage &getImage() const;

  // 受保护的事件处理函数 (Protected Event Handlers):
  // override 关键字确保我们正确地重写了基类的虚函数
 protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;    // 当窗口需要重绘时调用
  void resizeEvent(QResizeEvent *event) override;  // 当窗口大小改变时调用

  // 私有成员 (Private Members):
 private:
  // 私有辅助函数
  void drawLineTo(const QPoint &endPoint);                // 绘制一条线到指定点
  void resizeImage(QImage *image, const QSize &newSize);  // 调整图像大小

  bool drawing;     // 标记当前是否正在绘图
  int penWidth;     // 当前画笔的粗细
  QColor penColor;  // 当前画笔的颜色

  QPoint lastPoint;  // 记录上一个鼠标点的位置，用于绘制连续线条
  QImage image;      // 核心成员，作为“画纸”，所有内容都绘制在这上面
};

#endif  // DRAWINGCANVAS_H
