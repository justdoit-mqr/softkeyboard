#include "softkeyboard.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
    SoftKeyboard w;
    //w.setInputText("hello");
    w.show();

    return a.exec();
}
