#ifndef FORM_H
#define FORM_H

#include "softkeyboard.h"
#include <QWidget>
#include <QMouseEvent>

namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();
protected:
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void setLineEdit_2Text(QString text);

private:
    Ui::Form *ui;
    SoftKeyboard *softKeyboard;
};

#endif // FORM_H
