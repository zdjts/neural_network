#ifndef DRAWINGCANVAS_H
#define DRAWINGCANVAS_H

#include <QImage>
#include <QPoint>
#include <QWidget>


class QMouseEvent;
class QPaintEvent;
class QResizeEvent;

class DrawingCanvas : public QWidget {
  
  Q_OBJECT

 public:
  
  explicit DrawingCanvas(QWidget *parent = nullptr);

  
 public slots:
  void clearCanvas();              
  void setPenWidth(int newWidth);  

  
  const QImage &getImage() const;

  
  
 protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;    
  void resizeEvent(QResizeEvent *event) override;  

  
 private:
  
  void drawLineTo(const QPoint &endPoint);                
  void resizeImage(QImage *image, const QSize &newSize);  

  bool drawing;     
  int penWidth;     
  QColor penColor;  

  QPoint lastPoint;  
  QImage image;      
};

#endif  
