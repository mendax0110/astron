#include "ui/main_window.h"

#include <QApplication>
#include <QSurfaceFormat>

#include <QVTKOpenGLNativeWidget.h>

int main(int argc, char* argv[])
{
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
    QApplication application(argc, argv);
    application.setApplicationName("Astron");
    application.setOrganizationName("Astron");

    astron::MainWindow window;
    window.show();
    return application.exec();
}
