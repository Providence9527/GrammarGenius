#include "Genius.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Genius w;
    w.show();
    return a.exec();
}
