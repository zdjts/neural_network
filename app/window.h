#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <map>
#include <memory>
#include <string>

#include "../src/core/Graph.h"  
#include "../src/core/Node.h"   


class DrawingCanvas;
class QPushButton;
class QSlider;
class QLabel;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

 private slots:
  void recognizeImage();                
  void updatePenWidthLabel(int width);  

 private:
  
  void initializeNetwork();
  
  int predict(const Matrix &imageMatrix);

  
  DrawingCanvas *canvas;
  QPushButton *recognizeButton;
  QPushButton *clearButton;
  QSlider *penWidthSlider;
  QLabel *penWidthLabel;
  QLabel *resultLabel;

  
  std::unique_ptr<Graph> graph;          
  std::shared_ptr<Node> W1, b1, W2, b2;  
};

#endif  
