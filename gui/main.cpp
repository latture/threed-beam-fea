#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationName("Latture");
    app.setApplicationName("beam-fea");

    MainWindow mainwindow;
    mainwindow.show();

    return app.exec();
}