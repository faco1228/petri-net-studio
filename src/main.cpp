/**
 * @file main.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Application entry point — initializes QApplication and launches MainWindow.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. QApplication is constructed with command-line arguments
 *   2. Application metadata (name, version) is set for QSettings and about-dialogs
 *   3. MainWindow is created and shown
 *   4. The Qt event loop runs until the window is closed
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
