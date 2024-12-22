#ifndef MOUSE_DRAWING_WIDGET_H
#define MOUSE_DRAWING_WIDGET_H

#include <QWidget>
#include <QColor>
#include <QPoint>
#include <QPen>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QStack>
#include <QImage>
#include <QVector>

enum class PenStyle {
    Line,
    Circle,
    Square,
    Fill
};

struct DrawingAction {
    PenStyle penStyle;
    QPen pen;
    QVector<QPoint> points;
    QColor targetColor;
    QColor replacementColor;
};


class MouseDrawingWidget : public QWidget
{
    Q_OBJECT

public:
    MouseDrawingWidget(QWidget *parent = nullptr);
    ~MouseDrawingWidget() override;

public slots:
    void changePenStyle(int style);
    void changePenWidth(int width);
    void saveImage();
    void undo();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QColor penColor;
    PenStyle penStyle;
    QPoint lastPoint;
    bool isDrawing;
    QImage canvasImage;
    QPen pen;
    QRadioButton* lineButton;
    QRadioButton* circleButton;
    QRadioButton* squareButton;
    QRadioButton* fillButton;
    QButtonGroup* penStyleGroup;

    QVBoxLayout* mainLayout;
    QHBoxLayout* buttonLayout;
    QSpinBox* widthSpinBox;
    QPushButton* saveButton;
    QPushButton* undoButton;
    QStack<DrawingAction> drawingActions;
    QVector<QPoint> currentLine;

    void drawPoint(QPoint point);
    void drawLine(const QVector<QPoint>& points);
    void drawCircle(QPoint center);
    void drawSquare(QPoint center);
    void redrawAll();
    void floodFill(int x, int y, const QColor& targetColor, const QColor& replacementColor);
};

#endif // MOUSE_DRAWING_WIDGET_H
