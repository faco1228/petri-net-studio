/**
 * @file main.cpp
 * @brief Application entry point - initializes QApplication and launches MainWindow.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#include <QApplication>
#include "gui/main_window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("ICP Petri Net Editor");
    app.setApplicationVersion("1.0");

    MainWindow window;
    window.show();

    return app.exec();
}
