/****************************************************************************
*
* Copyright (C) 2016-2020 MiaoQingrui. All rights reserved.
* Author: 缪庆瑞 <justdoit_mqr@163.com>
*
****************************************************************************/
/*
 *@author: 缪庆瑞
 *@date:   2016.12.25
 *@update:  2020.05.09
 *@brief:  软键盘部件，实现中英文输入
 */
#ifndef SOFTKEYBOARD_H
#define SOFTKEYBOARD_H

#include <QWidget>
#include <QToolButton>
#include <QLabel>
#include <QLineEdit>
#include <QFile>
#include <QMessageBox>
#include <QStackedWidget>
#include <QMouseEvent>
#include <QPoint>

#define CANDIDATEWORDNUM 6   //默认候选词数量

class SoftKeyboard : public QWidget
{
    Q_OBJECT
public:
    explicit SoftKeyboard(QWidget *parent = 0);
    ~SoftKeyboard();

    //主要的对外接口
    void selectKeyboardStyle(int num);//选择键盘样式
    void setMoveEnabled(bool moveEnabled=true);//设置无边框窗口移动使能
    void showInputBufferArea(QString inputTitle=QString("Please input"),QString inputContent=QString());//显示输入缓存区域
    void hideInputBufferArea(QLineEdit *currLineEdit);//隐藏输入缓存区域

protected:
    //通过这三个事件处理函数实现无边框窗口的移动
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    void initStyleSheet();//初始化可选样式表，用于不同的皮肤展示
    void initNumberLetterBtn();//初始化数字字母按键
    void initSpecialBtn();//初始化特殊功能按键
    void setLetterLow();//设置小写字母显示
    void setLetterUpper();//设置大写字母显示
    void setSymbolsEN();//设置符号（英文状态）
    void setSymbolsCH();//设置符号（中文状态）

    void initInputBufferArea();//初始化输入缓存区
    void initFunctionAndCandidateArea();//初始化功能和候选区域
    void initKeysArea();//初始化按键区域

    void readDictionary();//读拼音字典，将汉字与拼音的对应存放到hash表中
    void splitPhrase(QString phrase,QString chinese);//拆分拼音词组
    void matchChinese(QString pinyin);//根据输入的拼音匹配中文
    void displayCandidateWord(int page);//显示指定页的候选词
    void hideCandidateArea();//隐藏中文输入显示区域

signals:
    void sendInputBufferAreaText(QString text);//以信号的形式将输入缓存区文本发出去

public slots:
    void candidateLetterChangedSlot(QString text);//候选字母改变响应槽
    void candidateWordBtnSlot();//候选词被点击的响应槽
    void candidateWordPrePageSlot();//候选词向前翻页
    void candidateWordNextPageSlot();//候选词向后翻页

    void numberLetterBtnSlot();//数字字母(符号)按键被点击的响应槽
    void changeUpperLowerSlot();//切换大小写，也可以切换数字字母与符号界面
    void deleteTextSlot();//删除输入
    void changeSkinSlot();//切换皮肤
    void changeLetterSymbolSlot();//数字字母与字符切换
    void spaceSlot();//空格被按下时的响应槽
    void changeChEnSlot();//中英文切换
    void enterSlot();//回车被按下的响应槽

    void clearAndCloseSlot();//清理并关闭键盘

private:
    QMultiHash<QString,QString> chinesePinyin;//使用哈希表来存放拼音汉字的键值对 一键多值
    QList<QString> hanzi;//存储匹配的汉字词

    /***************各种状态变量***************/
    //模式
    bool isENInput;//中英文输入模式
    bool isLetterInput;//数字字母或符号输入模式
    bool isLetterLower;//大小写模式
    int skinNum;//当前皮肤编号
    int pageCount;//候选词当前页
    //无边框窗口移动相关参数
    QPoint cursorGlobalPos;
    bool isMousePress;
    bool isMoveEnabled;

    /**************键盘输入缓存区**************/
    QWidget *inputBufferArea;
    QWidget *inputBufferWidget;
    QLabel *inputTitleLabel;
    QLineEdit *inputContentEdit;
    QLineEdit *currentLineEdit;//键盘当前的输入编辑框，可以接受外面传递的指针，默认为内置的inputContentEdit

    /***********键盘功能及候选词区域************/
    QStackedWidget *functionAndCandidateArea;
    QStringList functionAndCandidateAreaStyle;//功能和候选区区域的样式
    //功能区  后期可以添加各种功能配置的入口按钮
    QWidget *functionArea;
    QLabel *introduceLabel;
    //候选词区域 包括候选字母、候选词和翻页按钮
    QWidget *candidateArea;
    QLineEdit *candidateLetter;//中文输入时对应的字母显示
    QWidget *candidateWordArea;
    QToolButton *candidateWordBtn[CANDIDATEWORDNUM];//中文输入时的候选词
    QToolButton *prePageBtn;//前一页
    QToolButton *nextPageBtn;//后一页

    /***************键盘按键区域****************/
    QWidget *keysArea;//键盘的按键区域
    QStringList keysAreaStyle;//按键区域的样式
    //10个数字按键，26个字母按键,同时可以显示符号
    QToolButton *numberLetterBtn[36];
    //特殊功能按键
    QToolButton *upperOrLowerBtn;//大小写转换按键
    QToolButton *deleteBtn;//删除按键
    QToolButton *skinBtn;//切换皮肤
    QToolButton *letterOrSymbolBtn;//数字字母与符号切换按键
    QToolButton *commaBtn;//逗号按键
    QToolButton *spaceBtn;//空格按键
    QToolButton *periodBtn;//句号按键
    QToolButton *chOrEnBtn;//中英文切换按键
    QToolButton *enterBtn;//回车按键

};

#endif // SOFTKEYBOARD_H
