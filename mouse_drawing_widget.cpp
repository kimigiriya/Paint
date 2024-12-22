#include "mouse_drawing_widget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QColorDialog>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QStack>

MouseDrawingWidget::MouseDrawingWidget(QWidget *parent)
    : QWidget(parent)
{
    penColor = Qt::black;
    penStyle = PenStyle::Line;
    isDrawing = false;
    pen.setColor(penColor);
    pen.setWidth(2);

    // Создаем QImage для холста
    canvasImage = QImage(800, 600, QImage::Format_RGB32);
    canvasImage.fill(Qt::white);

    // Основной вертикальный макет
    mainLayout = new QVBoxLayout(this);

    // Создание макета для кнопок
    buttonLayout = new QHBoxLayout();

    // Создание radio кнопок
    lineButton = new QRadioButton("Линия", this);
    circleButton = new QRadioButton("Круглешки", this);
    squareButton = new QRadioButton("Квадратики", this);
    fillButton = new QRadioButton("Заливка", this);

    // Создаем группу для radio кнопок
    penStyleGroup = new QButtonGroup(this);
    penStyleGroup->addButton(lineButton, (int) PenStyle::Line);
    penStyleGroup->addButton(circleButton, (int) PenStyle::Circle);
    penStyleGroup->addButton(squareButton, (int) PenStyle::Square);
    penStyleGroup->addButton(fillButton, (int) PenStyle::Fill);

    lineButton->setChecked(true);

    buttonLayout->addWidget(lineButton);
    buttonLayout->addWidget(circleButton);
    buttonLayout->addWidget(squareButton);
    buttonLayout->addWidget(fillButton);

    // Создание виджета для выбора ширины кисти
    QLabel *widthLabel = new QLabel("Ширина:", this);
    widthSpinBox = new QSpinBox(this);
    widthSpinBox->setRange(1, 20);
    widthSpinBox->setValue(2); // Начальная ширина
    connect(widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MouseDrawingWidget::changePenWidth);

    buttonLayout->addWidget(widthLabel);
    buttonLayout->addWidget(widthSpinBox);

    // Создание кнопки "Сохранить"
    saveButton = new QPushButton("Сохранить", this);
    connect(saveButton, &QPushButton::clicked, this, &MouseDrawingWidget::saveImage);
    buttonLayout->addWidget(saveButton);
    // Made by kimi
    // Создание кнопки "Удалить"
    undoButton = new QPushButton("Удалить", this);
    connect(undoButton, &QPushButton::clicked, this, &MouseDrawingWidget::undo);
    buttonLayout->addWidget(undoButton);

    // Добавляем макет с кнопками в основной макет
    mainLayout->addLayout(buttonLayout);

    // Холст будет просто виджетом, поэтому добавляем его в основной макет
    mainLayout->addStretch();
    setLayout(mainLayout);

    // Подключение radio кнопок к слоту
    connect(penStyleGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
            [this](QAbstractButton* button) {
                changePenStyle(penStyleGroup->id(button));
            });
}

MouseDrawingWidget::~MouseDrawingWidget()
{
}

void MouseDrawingWidget::changePenStyle(int style)
{
    penStyle = static_cast<PenStyle>(style);
}

void MouseDrawingWidget::changePenWidth(int width)
{
    pen.setWidth(width);
}

void MouseDrawingWidget::saveImage() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Image", "", "PNG Files (*.png)");
    if(!fileName.isEmpty()) {
        if(canvasImage.save(fileName, "PNG")) {
            QMessageBox::information(this, "Сохранение", "Изображение сохранено успешно");
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось сохранить изображение");
        }
    }
}

void MouseDrawingWidget::undo() {
    if(!drawingActions.isEmpty()){
        drawingActions.pop();
        canvasImage.fill(Qt::white);
        redrawAll();
        update();
    }
}

void MouseDrawingWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDrawing = true;
        lastPoint = event->pos();
        currentLine.clear();
        currentLine.push_back(lastPoint);
    } else if (event->button() == Qt::RightButton) {
        QColor newColor = QColorDialog::getColor(penColor, this, "Выберите цвет для кисти");
        if (newColor.isValid()) {
            penColor = newColor;
            pen.setColor(penColor);
        }
    }

}

void MouseDrawingWidget::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && isDrawing) {
        QPoint currentPoint = event->pos();

        if (penStyle == PenStyle::Line) {
            QPainter painter(&canvasImage);
            QPen currentPen = pen;
            painter.setPen(currentPen);

            if (!currentLine.isEmpty()) {
                painter.drawLine(currentLine.last(), currentPoint);
            }
            currentLine.push_back(currentPoint);
        } else if(penStyle == PenStyle::Circle) {
            drawCircle(currentPoint);
        } else if (penStyle == PenStyle::Square) {
            drawSquare(currentPoint);
        }

        lastPoint = currentPoint;
    }
    update();
}

void MouseDrawingWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && isDrawing) {
        isDrawing = false;
        if(penStyle == PenStyle::Line) {
            DrawingAction action;
            action.penStyle = PenStyle::Line;
            action.points = currentLine;
            action.pen = pen;
            drawingActions.push(action);
        } else if(penStyle == PenStyle::Circle) {
            DrawingAction action;
            action.penStyle = PenStyle::Circle;
            action.points = {lastPoint};
            action.pen = pen;
            drawingActions.push(action);
        } else if(penStyle == PenStyle::Square) {
            DrawingAction action;
            action.penStyle = PenStyle::Square;
            action.points = {lastPoint};
            action.pen = pen;
            drawingActions.push(action);
        }


        currentLine.clear(); // Очищаем текущую линию
        lastPoint = QPoint();
    }
}

void MouseDrawingWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && penStyle == PenStyle::Fill) {
        QPoint clickedPoint = event->pos();
        QColor targetColor = canvasImage.pixelColor(clickedPoint);
        if (targetColor != penColor) {

            DrawingAction action;
            action.penStyle = PenStyle::Fill;
            action.targetColor = targetColor;
            action.replacementColor = penColor;
            action.points = {clickedPoint};
            drawingActions.push(action);

            floodFill(clickedPoint.x(), clickedPoint.y(), targetColor, penColor);
            update();
        }
    }
}

void MouseDrawingWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(0, 0, canvasImage);
}

void MouseDrawingWidget::drawLine(const QVector<QPoint>& points) {

}

void MouseDrawingWidget::drawPoint(QPoint point) {
    QPainter painter(&canvasImage);
    QPen currentPen = pen;
    painter.setPen(currentPen);
    painter.drawPoint(point);
}

void MouseDrawingWidget::drawCircle(QPoint center) {
    int radius = pen.width() * 2;
    QPainter painter(&canvasImage);
    QPen currentPen = pen;
    painter.setPen(currentPen);
    painter.drawEllipse(center.x() - radius, center.y() - radius,
                        radius * 2, radius * 2);
}

void MouseDrawingWidget::drawSquare(QPoint center) {
    int size = pen.width() * 5;
    QPainter painter(&canvasImage);
    QPen currentPen = pen;
    painter.setPen(currentPen);
    painter.drawRect(center.x() - size / 2, center.y() - size / 2, size, size);
}

void MouseDrawingWidget::redrawAll(){
    QPainter painter(&canvasImage);
    for(const auto& action : drawingActions) {
        painter.setPen(action.pen);

        if(action.penStyle == PenStyle::Line) {
            for (int i = 0; i < action.points.size() - 1; ++i) {
                painter.drawLine(action.points[i], action.points[i + 1]);
            }
        } else if (action.penStyle == PenStyle::Circle) {
            int radius = action.pen.width() * 2;
            painter.drawEllipse(action.points[0].x() - radius, action.points[0].y() - radius,
                                radius * 2, radius * 2);
        } else if (action.penStyle == PenStyle::Square) {
            int size = action.pen.width() * 5;
            painter.drawRect(action.points[0].x() - size / 2, action.points[0].y() - size / 2, size, size);
        } else if(action.penStyle == PenStyle::Fill) {
            floodFill(action.points[0].x(), action.points[0].y(), action.targetColor, action.replacementColor);
        }
    }
}

void MouseDrawingWidget::floodFill(int x, int y, const QColor& targetColor, const QColor& replacementColor) {
    if (x < 0 || x >= canvasImage.width() || y < 0 || y >= canvasImage.height()) {
        return;
    }

    if(canvasImage.pixelColor(x, y) != targetColor){
        return;
    }

    QStack<QPoint> stack;
    stack.push(QPoint(x, y));
    while (!stack.isEmpty())
    {
        QPoint current = stack.pop();

        if(canvasImage.pixelColor(current) == targetColor) {
            canvasImage.setPixelColor(current, replacementColor);

            stack.push(QPoint(current.x()+1, current.y()));
            stack.push(QPoint(current.x()-1, current.y()));
            stack.push(QPoint(current.x(), current.y()+1));
            stack.push(QPoint(current.x(), current.y()-1));
        }
    }
}
