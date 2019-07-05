#include "form.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
    Form w;
    w.show();

    return a.exec();
}
