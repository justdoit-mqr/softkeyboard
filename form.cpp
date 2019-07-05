#include "form.h"
#include "ui_form.h"
#include <QDebug>

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    softKeyboard = new SoftKeyboard();
    ui->lineEdit->installEventFilter(this);
    ui->lineEdit_2->installEventFilter(this);
}

Form::~Form()
{
    delete ui;
}
//事件过滤器处理函数
bool Form::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type()==QEvent::MouseButtonRelease)//事件类型
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);//将事件强制转换
        if(mouseEvent->button() == Qt::LeftButton)
        {
            QPoint point = this->pos();
            if(watched == ui->lineEdit)
            {
                disconnect(softKeyboard,SIGNAL(sendText(QString)),0,0);
                softKeyboard->setTextBufferAreaVisibled(false);
                softKeyboard->setCurrentLineEdit(ui->lineEdit);
                softKeyboard->resize(600,320);
                softKeyboard->move(point+ui->lineEdit->pos());
                softKeyboard->show();
            }
            else if(watched == ui->lineEdit_2)
            {
                disconnect(softKeyboard,SIGNAL(sendText(QString)),0,0);
                connect(softKeyboard,SIGNAL(sendText(QString)),this,SLOT(setLineEdit_2Text(QString)));
                softKeyboard->setTextBufferAreaVisibled(true);
                softKeyboard->setCurrentLineEdit(NULL);
                //softKeyboard->setTitleLabelText("please input:");
                softKeyboard->resize(600,320);
                softKeyboard->move(point+ui->lineEdit_2->pos());
                softKeyboard->show();
            }
        }
    }
    return QWidget::eventFilter(watched,event);//返回父类的事件过滤器
}

void Form::setLineEdit_2Text(QString text)
{
    ui->lineEdit_2->setText(text);
}

