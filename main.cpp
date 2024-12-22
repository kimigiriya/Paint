#include "mouse_drawing_widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MouseDrawingWidget window;
    window.setWindowTitle("KimiPaint");
    window.resize(800, 600);
    window.show();

    return app.exec();
}
