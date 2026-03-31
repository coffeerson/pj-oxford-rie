#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle(QStringLiteral("Oxford ICP133 RIE 控制软件"));
    w.resize(1200, 800);
    w.show();
    return a.exec();
}
