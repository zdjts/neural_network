#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>

#include "drawingcanvas.h"


const QImage &DrawingCanvas::getImage() const {
  return image;
}

DrawingCanvas::DrawingCanvas(QWidget *parent)
    : QWidget(parent), drawing(false), penWidth(5), penColor(Qt::black) {
  
  setAutoFillBackground(true);
  QPalette pal = palette();
  pal.setColor(QPalette::Window, Qt::white);
  setPalette(pal);

  
  
  setMouseTracking(true);
}


void DrawingCanvas::clearCanvas() {
  
  image.fill(qRgb(255, 255, 255));
  
  update();
}


void DrawingCanvas::setPenWidth(int newWidth) {
  if (newWidth > 0) {
    penWidth = newWidth;
  }
}


void DrawingCanvas::mousePressEvent(QMouseEvent *event) {
  
  if (event->button() == Qt::LeftButton) {
    lastPoint = event->pos();  
    drawing = true;            
  }
}


void DrawingCanvas::mouseMoveEvent(QMouseEvent *event) {
  
  if ((event->buttons() & Qt::LeftButton) && drawing) {
    drawLineTo(event->pos());  
  }
}


void DrawingCanvas::mouseReleaseEvent(QMouseEvent *event) {
  
  if (event->button() == Qt::LeftButton && drawing) {
    drawLineTo(event->pos());  
    drawing = false;           
  }
}


void DrawingCanvas::paintEvent(QPaintEvent *event) {
  
  QPainter painter(this);
  
  QRect dirtyRect = event->rect();
  
  painter.drawImage(dirtyRect, image, dirtyRect);
}


void DrawingCanvas::resizeEvent(QResizeEvent *event) {
  
  if (width() > image.width() || height() > image.height()) {
    resizeImage(&image, event->size());
  }
  
  QWidget::resizeEvent(event);
}


void DrawingCanvas::drawLineTo(const QPoint &endPoint) {
  
  QPainter painter(&image);
  
  painter.setPen(
      QPen(penColor, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  
  painter.drawLine(lastPoint, endPoint);

  
  
  int rad = (penWidth / 2) + 2;
  
  update(
      QRect(lastPoint, endPoint).normalized().adjusted(-rad, -rad, +rad, +rad));

  
  lastPoint = endPoint;
}


void DrawingCanvas::resizeImage(QImage *image, const QSize &newSize) {
  
  if (image->size() == newSize) return;

  
  QImage newImage(newSize, QImage::Format_RGB32);
  
  newImage.fill(qRgb(255, 255, 255));
  
  QPainter painter(&newImage);
  painter.drawImage(QPoint(0, 0), *image);
  
  *image = newImage;
}

