
// ===================================================================
// drawingcanvas.cpp
// 绘图画布组件的源文件
//
// 职责:
// 1. 实现头文件中声明的所有函数。
// 2. 封装所有绘图逻辑。
// ===================================================================
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>

#include "drawingcanvas.h"

// 新增: 获取当前画布图像
const QImage &DrawingCanvas::getImage() const {
  return image;
}
// 构造函数
DrawingCanvas::DrawingCanvas(QWidget *parent)
    : QWidget(parent), drawing(false), penWidth(5), penColor(Qt::black) {
  // 设置背景为白色，这样我们的“画纸”就有一个白色的底
  setAutoFillBackground(true);
  QPalette pal = palette();
  pal.setColor(QPalette::Window, Qt::white);
  setPalette(pal);

  // 允许捕获鼠标移动事件，即使没有按下鼠标按钮
  // 这对于实现悬停效果等很有用，但在这里不是必需的
  setMouseTracking(true);
}

// 公共槽函数: 清除画布
void DrawingCanvas::clearCanvas() {
  // 用白色填充整个 image
  image.fill(qRgb(255, 255, 255));
  // 调用 update() 来触发一次 paintEvent，让屏幕上的内容更新
  update();
}

// 公共槽函数: 设置画笔粗细
void DrawingCanvas::setPenWidth(int newWidth) {
  if (newWidth > 0) {
    penWidth = newWidth;
  }
}

// 事件处理: 鼠标按下
void DrawingCanvas::mousePressEvent(QMouseEvent *event) {
  // 只响应鼠标左键
  if (event->button() == Qt::LeftButton) {
    lastPoint = event->pos();  // 记录当前点为起始点
    drawing = true;            // 开始绘图
  }
}

// 事件处理: 鼠标移动
void DrawingCanvas::mouseMoveEvent(QMouseEvent *event) {
  // 如果鼠标左键被按住且处于绘图状态
  if ((event->buttons() & Qt::LeftButton) && drawing) {
    drawLineTo(event->pos());  // 从上一个点画线到当前点
  }
}

// 事件处理: 鼠标释放
void DrawingCanvas::mouseReleaseEvent(QMouseEvent *event) {
  // 如果是鼠标左键释放且刚刚完成了绘图
  if (event->button() == Qt::LeftButton && drawing) {
    drawLineTo(event->pos());  // 画下最后一笔
    drawing = false;           // 结束绘图
  }
}

// 事件处理: 绘制
void DrawingCanvas::paintEvent(QPaintEvent *event) {
  // 创建一个 QPainter，并指定绘图设备为当前窗口 (this)
  QPainter painter(this);
  // 获取需要重绘的区域，可以用于局部刷新优化
  QRect dirtyRect = event->rect();
  // 将 image 的相应区域绘制到窗口上
  painter.drawImage(dirtyRect, image, dirtyRect);
}

// 事件处理: 窗口大小调整
void DrawingCanvas::resizeEvent(QResizeEvent *event) {
  // 当窗口变大时，需要确保我们的“画纸”(QImage)也足够大
  if (width() > image.width() || height() > image.height()) {
    resizeImage(&image, event->size());
  }
  // 调用基类的实现来完成标准的 resize 处理
  QWidget::resizeEvent(event);
}

// 私有辅助函数: 绘制一条线
void DrawingCanvas::drawLineTo(const QPoint &endPoint) {
  // 创建一个 QPainter，但这次的绘图设备是我们的 QImage
  QPainter painter(&image);
  // 设置画笔属性
  painter.setPen(
      QPen(penColor, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  // 在 image 上绘制一条线
  painter.drawLine(lastPoint, endPoint);

  // 优化：只更新被修改的矩形区域，而不是整个窗口
  // 计算一个包含线条和笔宽的“脏矩形”
  int rad = (penWidth / 2) + 2;
  // schedule a paint event for the rectangle "r" on the widget
  update(
      QRect(lastPoint, endPoint).normalized().adjusted(-rad, -rad, +rad, +rad));

  // 更新 lastPoint，为下一次绘制做准备
  lastPoint = endPoint;
}

// 私有辅助函数: 调整图像大小
void DrawingCanvas::resizeImage(QImage *image, const QSize &newSize) {
  // 如果图像已经足够大，则无需操作
  if (image->size() == newSize) return;

  // 创建一个具有新尺寸的新图像
  QImage newImage(newSize, QImage::Format_RGB32);
  // 用白色填充新图像
  newImage.fill(qRgb(255, 255, 255));
  // 将旧图像的内容绘制到新图像上
  QPainter painter(&newImage);
  painter.drawImage(QPoint(0, 0), *image);
  // 用新图像替换旧图像
  *image = newImage;
}

