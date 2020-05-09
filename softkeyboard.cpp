/*
 *@author: 缪庆瑞
 *@date:   2016.12.25
 *@update:  2020.05.09
 *@brief:  软键盘部件，实现中英文输入
 */
#include "softkeyboard.h"
#include <QBoxLayout>
#include <QDebug>
#include <QDateTime>

#define PINYINFILEPATH  "./ChinesePinyin"

SoftKeyboard::SoftKeyboard(QWidget *parent) :
    QWidget(parent),cursorGlobalPos(0,0),isMousePress(false)
{
    /*设置键盘整体界面的最小大小，因为整体界面添加布局，布局的默认约束为SetDefaultConstraint
    这种约束只针对顶级窗口，会设置顶级窗口的最小大小为布局的minimumsize，而布局的最小大小是由内部的
    部件或子布局的推荐大小决定，所以设置窗口的最小大小，可以避免布局推荐大小过大，整个窗口无法缩小,只能
    通过setFixesize来固定不可变的大小*/
    this->setMinimumSize(450,300);
    this->resize(800,480);//默认大小
    this->setWindowFlags(Qt::FramelessWindowHint);//无边框
    this->setWindowModality(Qt::ApplicationModal);//应用模态
    //初始化ui显示
    this->initStyleSheet();
    this->initInputBufferArea();
    this->initFunctionAndCandidateArea();
    this->initKeysArea();
    this->selectKeyboardStyle(0);//选择皮肤
    this->setMoveEnabled();
    //整体垂直布局
    QVBoxLayout *globalVLayout = new QVBoxLayout(this);
    globalVLayout->setMargin(2);
    globalVLayout->setSpacing(0);
    globalVLayout->addWidget(inputBufferArea,1);
    globalVLayout->addWidget(functionAndCandidateArea,1);
    globalVLayout->addWidget(keysArea,5);

    readDictionary();//读拼音字典
    showInputBufferArea();
}

SoftKeyboard::~SoftKeyboard()
{

}
/*
 *@brief:   选择键盘样式，所有按键及功能候选区域背景的样式在此设置
 *@author:  缪庆瑞
 *@date:    2020.05.09
 *@param:   num:确定用哪一套样式（皮肤）
 */
void SoftKeyboard::selectKeyboardStyle(int num)
{
    if(num>=keysAreaStyle.size())
    {
        qDebug()<<"selectKeyboardStyle():The skin of the specified index does not exist.";
        return;//避免超出list长度，程序异常
    }
    skinNum = num;
    //设置按键区域的样式
    keysArea->setStyleSheet(keysAreaStyle.at(num));
    //设置功能和候选区区域的样式
    functionAndCandidateArea->setStyleSheet(functionAndCandidateAreaStyle.at(num));
}
/*
 *@brief:   设置无边框窗口是否可以移动
 *@author:  缪庆瑞
 *@date:    2019.7.5
 *@param:   moveEnabled:移动使能
 */
void SoftKeyboard::setMoveEnabled(bool moveEnabled)
{
    isMoveEnabled = moveEnabled;
}
/*
 *@brief:   显示输入缓存区
 * 该接口主要应用场景：点击某一个编辑框，弹出软键盘(可能会遮挡点击的编辑框)，键盘的输入内容会临时
 * 存放到内置的缓存区(标题和初始内容可通过该接口设置)，在点击回车时把缓存区的内容通过信号的形式发
 * 送出来，传递给外部的编辑框。
 *@author:  缪庆瑞
 *@date:    2020.05.09
 *@param:   inputTitle:输入缓存区标题
 *@param:   inputContent:输入缓存区的初始内容
 */
void SoftKeyboard::showInputBufferArea(QString inputTitle, QString inputContent)
{
    inputBufferArea->setVisible(true);
    inputTitleLabel->setText(inputTitle);
    inputContentEdit->setText(inputContent);
    inputContentEdit->setFocus();
    currentLineEdit = inputContentEdit;//将内置编辑框设置为当前编辑框
}
/*
 *@brief:   隐藏输入缓存区
 * 该接口主要应用场景：点击某一个编辑框，编辑框预计不会被软键盘遮挡，在弹出键盘之前调用该接口传递点击
 * 的编辑框指针，这样键盘输入的内容会直接显示在外部的编辑框中，不再需要使用信号传递
 *@author:  缪庆瑞
 *@date:    2020.05.09
 *@param:   currLineEdit:外部的编辑框指针
 */
void SoftKeyboard::hideInputBufferArea(QLineEdit *currLineEdit)
{
    inputBufferArea->setVisible(false);
    currentLineEdit = currLineEdit;
}
/*
 *@brief:   鼠标按下事件处理
 *@author:  缪庆瑞
 *@date:    2019.7.5
 */
void SoftKeyboard::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)//左键按下
    {
        isMousePress = true;
        cursorGlobalPos =event->globalPos();//获取鼠标按下时的全局位置
        //qDebug()<<"mouse press:"<<cursorGlobalPos;
    }
}
/*
 *@brief:   鼠标移动事件处理
 *@author:  缪庆瑞
 *@date:    2019.7.5
 */
void SoftKeyboard::mouseMoveEvent(QMouseEvent *event)
{
    //不可以用event.button()==Qt::LeftButton来判断，因为在鼠标移动事件里该方法一直返回Qt::NoButton
    if(event->buttons()&Qt::LeftButton && isMousePress && isMoveEnabled)
    {
        //qDebug()<<"mouse move:"<<event->globalPos();
        QPoint position = this->pos() + event->globalPos() - cursorGlobalPos;
        move(position.x(), position.y());
        cursorGlobalPos = event->globalPos();
    }
}
/*
 *@brief:   鼠标释放事件处理
 *@author:  缪庆瑞
 *@date:    2019.7.5
 */
void SoftKeyboard::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)//左键按下
    {
        isMousePress = false;
        //qDebug()<<"mouse release:";
    }
}
/*
 *@brief:   初始化样式表，即用于界面皮肤选择
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::initStyleSheet()
{
    /*皮肤1:雅黑*/
    //按键区域样式
    keysAreaStyle.append(
                ".QWidget{background-color:#1E1E1E;}"
                "QToolButton{background-color:#4E4E4E;color:#E6E6E6;border-radius:4px;}"
                "QToolButton#specialKeyStyle{background-color:#2D2D2D;border-radius:2px;}");
    //功能和候选区区域样式 目前与按键区域样式一致
    functionAndCandidateAreaStyle.append(
                "QWidget{background-color:#1E1E1E;color:#E6E6E6;}"
                "QToolButton{border-style:none;}"
                "QToolButton#pageBtn{font-weight:bold;}"
                "QToolButton#pageBtn:disabled{color:#6c97b4;}"
                "QLineEdit{background-color:#4E4E4E;}");
    /*皮肤2:简白*/
    keysAreaStyle.append(
                ".QWidget{background-color:#D8D8D8;}"
                "QToolButton{background-color:#FFFFFF;color:black;border-radius:6px;}"
                "QToolButton#specialKeyStyle{background-color:#EBEBEB;border-radius:6px;}");
    functionAndCandidateAreaStyle.append(
                "QWidget{background-color:#D8D8D8;color:black;}"
                "QToolButton{border-style:none;}"
                "QToolButton#pageBtn{font-weight:bold;}"
                "QToolButton#pageBtn:disabled{color:#6c97b4;}"
                "QLineEdit{background-color:white;}");
    /*皮肤3:魅紫*/
    keysAreaStyle.append(
                ".QWidget{background-color:#190724;}"
                "QToolButton{background-color:#272A5E;color:#68CBF2;border-radius:4px;}"
                "QToolButton#specialKeyStyle{background-color:#241D48;border-radius:2px;}");
    functionAndCandidateAreaStyle.append(
                "QWidget{background-color:#190724;color:#68CBF2;}"
                "QToolButton{border-style:none;}"
                "QToolButton#pageBtn{font-weight:bold;}"
                "QToolButton#pageBtn:disabled{color:#6c97b4;}"
                "QLineEdit{background-color:#272A5E;}");
}
/*
 *@brief:   初始化软键盘的数字字母(字符)按键
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::initNumberLetterBtn()
{
    //以下36个按键，仅作为普通输入，无其他功能，所以连接一个槽函数
    for(int i=0;i<36;i++)//为10个数字，26个字母按键申请空间,连接信号与槽
    {
        numberLetterBtn[i] = new QToolButton();
        numberLetterBtn[i]->setToolButtonStyle(Qt::ToolButtonTextOnly);
        numberLetterBtn[i]->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
        connect(numberLetterBtn[i],SIGNAL(clicked()),this,SLOT(numberLetterBtnSlot()));
    }
}
/*
 *@brief:   初始化软键盘的特殊功能按键,诸如大小写切换/删除等
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::initSpecialBtn()
{
    upperOrLowerBtn = new QToolButton();
    upperOrLowerBtn->setObjectName("specialKeyStyle");
    upperOrLowerBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    upperOrLowerBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    upperOrLowerBtn->setText("A/a");
    connect(upperOrLowerBtn,SIGNAL(clicked()),this,SLOT(changeUpperLowerSlot()));

    deleteBtn = new QToolButton();
    deleteBtn->setObjectName("specialKeyStyle");
    deleteBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    deleteBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    deleteBtn->setText("del");
    deleteBtn->setAutoRepeatDelay(300);
    deleteBtn->setAutoRepeatInterval(60);
    deleteBtn->setAutoRepeat(true);//启用自动重复功能，实现长按连续动作
    connect(deleteBtn,SIGNAL(clicked(bool)),this,SLOT(deleteTextSlot()));

    skinBtn = new QToolButton();
    skinBtn->setObjectName("specialKeyStyle");
    skinBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    skinBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    skinBtn->setText("skin");
    connect(skinBtn,SIGNAL(clicked()),this,SLOT(changeSkinSlot()));

    letterOrSymbolBtn = new QToolButton();
    letterOrSymbolBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    letterOrSymbolBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    letterOrSymbolBtn->setText("abc");
    connect(letterOrSymbolBtn,SIGNAL(clicked()),this,SLOT(changeLetterSymbolSlot()));

    commaBtn = new QToolButton();
    commaBtn->setMinimumSize(36,16);//设置最小大小，主要是为了清掉布局的默认最佳大小
    commaBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    commaBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    commaBtn->setText(",");
    connect(commaBtn,SIGNAL(clicked()),this,SLOT(numberLetterBtnSlot()));

    spaceBtn = new QToolButton();
    spaceBtn->setText(" ");
    spaceBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    spaceBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    connect(spaceBtn,SIGNAL(clicked()),this,SLOT(spaceSlot()));

    periodBtn = new QToolButton();
    periodBtn->setMinimumSize(36,16);
    periodBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    periodBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    periodBtn->setText(".");
    connect(periodBtn,SIGNAL(clicked()),this,SLOT(numberLetterBtnSlot()));

    chOrEnBtn = new QToolButton();
    chOrEnBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    chOrEnBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    chOrEnBtn->setText("CH");
    isENInput = false;//初始化为中文输入
    connect(chOrEnBtn,SIGNAL(clicked()),this,SLOT(changeChEnSlot()));

    enterBtn = new QToolButton();
    enterBtn->setObjectName("specialKeyStyle");
    enterBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    enterBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    enterBtn->setText("  Enter  ");
    connect(enterBtn,SIGNAL(clicked()),this,SLOT(enterSlot()));
}
/*
 *@brief:   设置小写字母显示
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::setLetterLow()
{
    this->isLetterInput = true;
    this->isLetterLower = true;
    this->letterOrSymbolBtn->setText("abc");
    QStringList letterLowList;
    letterLowList<<"1"<<"2"<<"3"<<"4"<<"5"<<"6"<<"7"<<"8"<<"9"<<"0"
                 <<"q"<<"w"<<"e"<<"r"<<"t"<<"y"<<"u"<<"i"<<"o"<<"p"
                 <<"a"<<"s"<<"d"<<"f"<<"g"<<"h"<<"j"<<"k"<<"l"
                 <<"z"<<"x"<<"c"<<"v"<<"b"<<"n"<<"m";
    for(int i=0;i<36;i++)
    {
        numberLetterBtn[i]->setText(letterLowList.at(i));
    }
}
/*
 *@brief:   设置大写字母显示
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::setLetterUpper()
{
    this->isLetterInput = true;
    this->isLetterLower = false;
    this->letterOrSymbolBtn->setText("abc");
    QStringList letterUpperList;
    letterUpperList<<"1"<<"2"<<"3"<<"4"<<"5"<<"6"<<"7"<<"8"<<"9"<<"0"
                   <<"Q"<<"W"<<"E"<<"R"<<"T"<<"Y"<<"U"<<"I"<<"O"<<"P"
                   <<"A"<<"S"<<"D"<<"F"<<"G"<<"H"<<"J"<<"K"<<"L"
                   <<"Z"<<"X"<<"C"<<"V"<<"B"<<"N"<<"M";
    for(int i=0;i<36;i++)
    {
        numberLetterBtn[i]->setText(letterUpperList.at(i));
    }
}
/*
 *@brief:   设置符号显示，包含所有可输入符号(英文状态)
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::setSymbolsEN()
{
    this->isLetterInput = false;
    this->letterOrSymbolBtn->setText(":;?");
    QStringList symbolsENList;
    symbolsENList<<"!"<<"@"<<"#"<<"$"<<"%"<<"^"<<"&&"<<"*"<<"("<<")"
               <<"["<<"]"<<"{"<<"}"<<"<"<<">"<<"+"<<"-"<<"_"<<"="
               <<"`"<<"~"<<"|"<<"\\"<<"'"<<"\""<<":"<<";"<<"?"
               <<"/"<<"..."<<"---"<<"www."<<".com"<<".cn"<<".net";
    for(int i=0;i<36;i++)
    {
        numberLetterBtn[i]->setText(symbolsENList.at(i));
    }
}
/*
 *@brief:   设置符号显示，包含所有可输入符号(中文状态)
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::setSymbolsCH()
{
    this->isLetterInput = false;
    this->letterOrSymbolBtn->setText(":;?");
    QStringList symbolsCHList;
    //为了兼容Qt4  中文状态下特有的字符用fromUtf8转换,避免使用tr()影响国际化翻译
    symbolsCHList<<QString::fromUtf8("！")<<"@"<<"#"<<QString::fromUtf8("￥")<<"%"<<QString::fromUtf8("…")<<"&&"<<"*"<<QString::fromUtf8("（")<<QString::fromUtf8("）")
            <<QString::fromUtf8("【")<<QString::fromUtf8("】")<<"{"<<"}"<<QString::fromUtf8("《")<<QString::fromUtf8("》")<<"+"<<"-"<<QString::fromUtf8("—")<<"="
            <<"."<<"~"<<"|"<<QString::fromUtf8("、")<<QString::fromUtf8("’")<<QString::fromUtf8("‘")<<QString::fromUtf8("“")<<QString::fromUtf8("”")<<QString::fromUtf8("：")
            <<QString::fromUtf8("；")<<QString::fromUtf8("？")<<QString::fromUtf8("♀")<<QString::fromUtf8("℃")<<QString::fromUtf8("★")<<QString::fromUtf8("→")<<QString::fromUtf8("←");
    for(int i=0;i<36;i++)
    {
        numberLetterBtn[i]->setText(symbolsCHList.at(i));
    }
}
/*
 *@brief:   初始化键盘输入缓存区，临时存放显示输入的内容
 *@author:  缪庆瑞
 *@date:    2019.7.5
 */
void SoftKeyboard::initInputBufferArea()
{
    //输入缓存标题
    inputTitleLabel = new QLabel();
    inputTitleLabel->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred);
    //输入缓存内容编辑框
    inputContentEdit = new QLineEdit();
    inputContentEdit->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    currentLineEdit = inputContentEdit;//默认将内置编辑框设置为当前编辑框
    //输入缓存部件
    inputBufferWidget = new QWidget();
    //inputBufferWidget->setStyleSheet(INPUT_BUFFER_AREA_STYLE);
    QHBoxLayout *hBoxLayout = new QHBoxLayout(inputBufferWidget);
    hBoxLayout->setContentsMargins(10,6,10,6);
    hBoxLayout->setSpacing(15);
    hBoxLayout->addWidget(inputTitleLabel);
    hBoxLayout->addWidget(inputContentEdit);
    //输入缓存区
    inputBufferArea = new QWidget();
    QHBoxLayout *inputBufferAreaLayout = new QHBoxLayout(inputBufferArea);
    inputBufferAreaLayout->setMargin(0);
    inputBufferAreaLayout->addStretch(1);
    inputBufferAreaLayout->addWidget(inputBufferWidget,2);
    inputBufferAreaLayout->addStretch(1);

}
/*
 *@brief:   初始化功能和候选词区域,该区域使用栈部件,同一时刻只显示一个区域
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::initFunctionAndCandidateArea()
{
    /*****************功能区*****************/
    //介绍
    introduceLabel = new QLabel();
    introduceLabel->setAlignment(Qt::AlignCenter);
    introduceLabel->setText(QString::fromUtf8("欢迎使用中文输入软键盘"));

    functionArea = new QWidget();
    QHBoxLayout *functionAreaLayout = new QHBoxLayout(functionArea);
    functionAreaLayout->setContentsMargins(8,0,8,0);
    functionAreaLayout->addWidget(introduceLabel);
    /*****************候选区*****************/
    //候选字母显示框
    candidateLetter = new QLineEdit();
    candidateLetter->setFrame(false);
    candidateLetter->setEnabled(false);
    candidateLetter->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);//水平方向固定大小
    connect(candidateLetter,SIGNAL(textChanged(QString)),this,SLOT(candidateLetterChangedSlot(QString)));
    //候选词、翻页按钮显示区域
    candidateWordArea = new QWidget();
    QHBoxLayout *candidateWordAreaLayout = new QHBoxLayout(candidateWordArea);
    candidateWordAreaLayout->setMargin(0);
    for(int i=0;i<CANDIDATEWORDNUM;i++)
    {
        candidateWordBtn[i] = new QToolButton();
        candidateWordBtn[i]->setToolButtonStyle(Qt::ToolButtonTextOnly);
        candidateWordBtn[i]->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);   
        connect(candidateWordBtn[i],SIGNAL(clicked()),this,SLOT(candidateWordBtnSlot()));
        candidateWordAreaLayout->addWidget(candidateWordBtn[i]);
    }
    prePageBtn = new QToolButton();//上一页
    prePageBtn->setObjectName("pageBtn");
    prePageBtn->setText("   <   ");
    prePageBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    prePageBtn->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred);
    connect(prePageBtn,SIGNAL(clicked()),this,SLOT(candidateWordPrePageSlot()));
    nextPageBtn = new QToolButton();//下一页
    nextPageBtn->setObjectName("pageBtn");
    nextPageBtn->setText("   >   ");
    nextPageBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    nextPageBtn->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred);
    connect(nextPageBtn,SIGNAL(clicked()),this,SLOT(candidateWordNextPageSlot()));
    candidateWordAreaLayout->addWidget(prePageBtn);
    candidateWordAreaLayout->addWidget(nextPageBtn);

    candidateArea = new QWidget();
    QVBoxLayout *vBoxLayout = new QVBoxLayout(candidateArea);
    vBoxLayout->setMargin(0);
    vBoxLayout->setSpacing(0);
    vBoxLayout->addWidget(candidateLetter);
    vBoxLayout->addWidget(candidateWordArea);
    /***************栈部件存放功能区和候选区******************/
    functionAndCandidateArea = new QStackedWidget();
    functionAndCandidateArea->addWidget(functionArea);
    functionAndCandidateArea->addWidget(candidateArea);
}
/*
 *@brief:   初始化键盘按键区域
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::initKeysArea()
{
    initNumberLetterBtn();//初始化10个数字，26个字母或字符按键
    initSpecialBtn();//初始化特殊按键
    setLetterLow();//默认显示字母界面 小写

    //布局第一排按键 10个数字
    QHBoxLayout *firstRowHLayout = new QHBoxLayout();
    for(int i=0;i<10;i++)
    {
        firstRowHLayout->addWidget(numberLetterBtn[i]);
    }
    //布局第二排按键
    QHBoxLayout *secondRowHLayout = new QHBoxLayout();
    for(int i=10;i<20;i++)
    {
        secondRowHLayout->addWidget(numberLetterBtn[i]);
    }
    //布局第三排按键
    QHBoxLayout *thirdRowHLayout = new QHBoxLayout();
    thirdRowHLayout->setContentsMargins(20,0,20,0);
    for(int i=20;i<29;i++)
    {
        thirdRowHLayout->addWidget(numberLetterBtn[i]);
    }
    //布局第四排按键
    QHBoxLayout *fourthRowHLayout = new QHBoxLayout();
    fourthRowHLayout->addWidget(upperOrLowerBtn);//大小写切换按键
    for(int i =29;i<36;i++)
    {
        fourthRowHLayout->addWidget(numberLetterBtn[i]);
    }
    fourthRowHLayout->addWidget(deleteBtn);//删除按键
    //布局第五排按键，基本为特殊功能按键
    QHBoxLayout *fifthRowHLayout = new QHBoxLayout();
    fifthRowHLayout->addWidget(skinBtn,2);
    fifthRowHLayout->addWidget(letterOrSymbolBtn,1);
    fifthRowHLayout->addWidget(commaBtn,1);
    fifthRowHLayout->addWidget(spaceBtn,4);
    fifthRowHLayout->addWidget(periodBtn,1);
    fifthRowHLayout->addWidget(chOrEnBtn,1);
    fifthRowHLayout->addWidget(enterBtn,2);

    keysArea = new QWidget();
    QVBoxLayout *vBoxlayout = new QVBoxLayout(keysArea);
    vBoxlayout->setContentsMargins(8,2,8,8);
    vBoxlayout->addLayout(firstRowHLayout);
    vBoxlayout->addLayout(secondRowHLayout);
    vBoxlayout->addLayout(thirdRowHLayout);
    vBoxlayout->addLayout(fourthRowHLayout);
    vBoxlayout->addLayout(fifthRowHLayout);
}
/*
 *@brief:   读拼音字典，将汉字与对应拼音存放到hash表中,也可以是QMap中，但不考虑排列顺序时，hash更快
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::readDictionary()
{
    //通过打印时间测试读拼音文件的效率 8G内存windows测试大约用1/10s
    qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd HH:m:s:z");
    QFile pinyinFile(PINYINFILEPATH);
    if(!pinyinFile.open(QIODevice::ReadOnly))//拼音文件打开失败提示
    {
        QMessageBox::critical(this,"Open File Failed",QString::fromUtf8("无法打开拼音文件。。。"));
    }
    else
    {
        QRegExp regExp("[a-z']+");//正则表达式，匹配1个或多个由a-z及 ' 组成的字母串，默认区分大小写
        QString lineText;//存放读取的一行数据 汉字-拼音
        QString linePinyin;//存放正则表达式匹配的拼音
        QString lineChinese;//存放拼音对应的汉字
        int pinyinPosition;//每一行匹配拼音的位置
        while(!pinyinFile.atEnd())//while循环读取拼音文件，直到读完
        {
            lineText = QString(QString::fromUtf8(pinyinFile.readLine()));
            pinyinPosition=regExp.indexIn(lineText,0);//获取读取行的文本中匹配正则表达式的位置
            linePinyin = regExp.cap(0);//regExp.cap(0)表示完整正则表达式的匹配
            lineChinese = lineText.left(pinyinPosition);//lineText.left(n)可以获取左边那个字符即对应的汉字
            if(linePinyin.contains("'"))//如果有单引号表示是词组，则进行拆分词组
            {
                splitPhrase(linePinyin,lineChinese);
            }
            else//单个汉字
            {
                chinesePinyin.insert(linePinyin,lineChinese);//往哈希表插入键值对
            }
        }
    }
    qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd HH:m:s:z");
    //qDebug()<<chinesePinyin.size();
}
/*
 *@brief:   拆分拼音词组，拼音字典文件词组用'分割，如"ai'qing"该函数的功能便是去掉'，
 * 将简拼、全拼存放到哈希表中
 *@author:  缪庆瑞
 *@date:    2017.2.7
 *@param:   phrase:要处理的拼音词组
 *@param:   chinese:拼音对应的汉字
 */
void SoftKeyboard::splitPhrase(QString phrase,QString chinese)
{
    int count = phrase.count("'");
    if(count==1)//两个汉字
    {
        int index=phrase.indexOf("'");
        QString pinyin1=phrase.left(1);//两字首字母简拼 例aq
        pinyin1.append(phrase.at(index+1));
        chinesePinyin.insert(pinyin1,chinese);
        QString pinyin2=phrase.left(index);//全拼+首字母 aiq
        pinyin2.append(phrase.at(index+1));
        if(pinyin2!=pinyin1)//避免同一词组键值对插入哈希表多次 例如 e'xi
        {
            chinesePinyin.insert(pinyin2,chinese);
        }
        QString pinyin3=phrase.remove("'");//全拼 aiqing
        if(pinyin3!=pinyin2)
        {
             chinesePinyin.insert(pinyin3,chinese);
        }
    }
    else if(count==2)//三个汉字
    {
        int index1=phrase.indexOf("'");
        int index2=phrase.indexOf("'",index1+1);
        QString pinyin1=phrase.left(1);//三字首字母简拼
        pinyin1.append(phrase.at(index1+1));
        pinyin1.append(phrase.at(index2+1));
        chinesePinyin.insert(pinyin1,chinese);
        QString pinyin2=phrase.left(index1);//全拼+首字母+首字母
        pinyin2.append(phrase.at(index1+1));
        pinyin2.append(phrase.at(index2+1));
        if(pinyin2!=pinyin1)//避免同一词组键值对插入哈希表多次 例如 e'xi
        {
            chinesePinyin.insert(pinyin2,chinese);
        }
        QString pinyin3=phrase.left(index2);//全拼+全拼+首字母
        pinyin3.append(phrase.at(index2+1));
        pinyin3.remove("'");
        if(pinyin3!=pinyin2)//避免同一词组键值对插入哈希表多次 例如 e'xi
        {
            chinesePinyin.insert(pinyin3,chinese);
        }
        QString pinyin4=phrase.remove("'");//全拼
        if(pinyin4!=pinyin3)//避免同一词组键值对插入哈希表多次 例如 e'xi
        {
            chinesePinyin.insert(pinyin4,chinese);
        }
    }
    else if(count==3)//四个汉字
    {
        int index1=phrase.indexOf("'");
        int index2=phrase.indexOf("'",index1+1);
        int index3=phrase.indexOf("'",index2+1);
        QString pinyin1=phrase.left(1);//四字首字母简拼
        pinyin1.append(phrase.at(index1+1));
        pinyin1.append(phrase.at(index2+1));
        pinyin1.append(phrase.at(index3+1));
        chinesePinyin.insert(pinyin1,chinese);
        QString pinyin2=phrase.left(index1);//全拼+首字母+首字母+首字母
        pinyin2.append(phrase.at(index1+1));
        pinyin2.append(phrase.at(index2+1));
        pinyin2.append(phrase.at(index3+1));
        if(pinyin2!=pinyin1)//避免同一词组键值对插入哈希表多次 例如 e'xing'xun'huan
        {
            chinesePinyin.insert(pinyin2,chinese);
        }
        QString pinyin3=phrase.left(index2);//全拼+全拼+首字母+首字母
        pinyin3.append(phrase.at(index2+1));
        pinyin3.append(phrase.at(index3+1));
        pinyin3.remove("'");
        if(pinyin3!=pinyin2)//避免同一词组键值对插入哈希表多次 例如 e'xing'xun'huan
        {
            chinesePinyin.insert(pinyin3,chinese);
        }
        QString pinyin4=phrase.left(index3);//全拼+全拼+全拼+首字母
        pinyin4.append(phrase.at(index3+1));
        pinyin4.remove("'");
        if(pinyin4!=pinyin3)//避免同一词组键值对插入哈希表多次 例如 e'xing'xun'huan
        {
            chinesePinyin.insert(pinyin4,chinese);
        }
        QString pinyin5=phrase.remove("'");//全拼
        if(pinyin5!=pinyin4)//避免同一词组键值对插入哈希表多次 例如 e'xing'xun'huan
        {
            chinesePinyin.insert(pinyin5,chinese);
        }
    }
}
/*
 *@brief:   根据输入的拼音匹配中文
 *@author:  缪庆瑞
 *@date:    2017.1.1
 *@param:   pinyin:输入的拼音
 */
void SoftKeyboard::matchChinese(QString pinyin)
{
    hanzi.clear();//每次匹配中文都先清空之前的列表
    //哈希表chinesePinyin中存放着拼音-汉字的键值对（一键多值），获取对应拼音的汉字列表
    hanzi = chinesePinyin.values(pinyin);
    //qDebug()<<hanzi;
    pageCount = 1;//每次匹配，候选词初始化显示第一页
}
/*
 *@brief:   显示指定页的候选词
 *@author:  缪庆瑞
 *@date:    2017.1.1
 *@param:   page:指定页,从1开始
 */
void SoftKeyboard::displayCandidateWord(int page)
{
    int hanziCount = hanzi.size();//匹配的汉字列表个数
    if(page == 1)//当显示第一页时向前翻页按钮不可点击
    {
        prePageBtn->setEnabled(false);
    }
    else
    {
        prePageBtn->setEnabled(true);
    }
    if(page*CANDIDATEWORDNUM>=hanziCount)
    {
        nextPageBtn->setEnabled(false);
    }
    else
    {
        nextPageBtn->setEnabled(true);
    }
    /*因为哈希表获取指定的key对应的值列表时,是按照后插入的先获取(后进先出),
     * 所以常见字在后面,这里处理为从列表的最后一个反向获取显示
     */
    int num = hanziCount-1-(page-1)*CANDIDATEWORDNUM;
    for(int i=0;i<CANDIDATEWORDNUM;i++)//每页默认6个候选字词
    {
        if(num>=i)
        {
            candidateWordBtn[i]->setText(hanzi.at(num-i));
            candidateWordBtn[i]->setEnabled(true);
        }
        else
        {
            candidateWordBtn[i]->setText(" ");//清除上一页的缓存
            candidateWordBtn[i]->setEnabled(false);
        }
    }
}
/*
 *@brief:   隐藏中文输入的候选区域
 *@author:  缪庆瑞
 *@date:    2017.1.4
 */
void SoftKeyboard::hideCandidateArea()
{
    candidateLetter->clear();//清空候选字母
    functionAndCandidateArea->setCurrentWidget(functionArea);//显示功能区
}
/*
 *@brief:   中文输入时候选字母区域根据内容改变文本框的大小
 *@author:  缪庆瑞
 *@date:    2020.05.09
 *@param:   text:即改变后的文本内容
 */
void SoftKeyboard::candidateLetterChangedSlot(QString text)
{
    //根据输入的内容自动改变文本区域大小
    int width = candidateLetter->fontMetrics().width(text)+6;
    candidateLetter->setFixedWidth(width);
}
/*
 *@brief:   候选词被点击的响应槽
 *@author:  缪庆瑞
 *@date:    2016.12.25
 */
void SoftKeyboard::candidateWordBtnSlot()
{
    QToolButton *clickedBtn = qobject_cast<QToolButton *>(sender());//获取信号发送者的对象
    currentLineEdit->insert(clickedBtn->text());
    hideCandidateArea();//隐藏中文候选区域
}
/*
 *@brief:   候选词向前翻页
 *@author:  缪庆瑞
 *@date:    2016.12.29
 */
void SoftKeyboard::candidateWordPrePageSlot()
{
    pageCount--;
    this->displayCandidateWord(pageCount);
}
/*
 *@brief:   候选词向后翻页
 *@author:  缪庆瑞
 *@date:    2016.12.29
 */
void SoftKeyboard::candidateWordNextPageSlot()
{
    pageCount++;
    this->displayCandidateWord(pageCount);
}
/*
 *@brief:   字母(符号)按键被点击的响应槽
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::numberLetterBtnSlot()
{
    QToolButton *clickedBtn = qobject_cast<QToolButton *>(sender());//获取信号发送者的对象
    //if(isENInput||!isLetterInput)//英文输入模式或者字符输入界面
    if(isENInput)//英文输入
    {
        if(clickedBtn->text()=="&&")//因为可显示控件把&符号当成快捷键标志，一个不显示，所以这个要做下特别处理
        {
            currentLineEdit->insert("&");
        }
        else
        {
            currentLineEdit->insert(clickedBtn->text());//文本输入框插入字母或符号
        }
    }
    else  //中文输入模式 键入的字母放在第二部分输入显示区域的候选字母按钮上
    {
        functionAndCandidateArea->setCurrentWidget(candidateArea);
        candidateLetter->insert(clickedBtn->text());//候选字母输入框插入字母
        this->matchChinese(candidateLetter->text());//匹配中文
        this->displayCandidateWord(pageCount);//显示候选词，默认第一页
    }
}
/*
 *@brief:   大小写切换 也可以切换数字字母与字符界面
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::changeUpperLowerSlot()
{
    if(isLetterInput)//当前为字母界面
    {
        if(isLetterLower)//当前为小写状态
        {
            this->setLetterUpper();//设置为大写状态
        }
        else
        {
            this->setLetterLow();//设置为小写状态
        }
    }
    else  //当前为数字字符界面,则返回之前的大小写字母界面
    {
        if(isLetterLower)
        {
            this->setLetterLow();
        }
        else
        {
            this->setLetterUpper();
        }
    }
}
/*
 *@brief:   删除输入内容
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::deleteTextSlot()
{
    if(functionAndCandidateArea->currentWidget() == candidateArea)
    {
        candidateLetter->backspace();//删除选中文本或光标前的一个字符，默认光标在最后
        if(candidateLetter->text().isEmpty())//删完了
        {
            hideCandidateArea();
        }
        else
        {
            matchChinese(candidateLetter->text());//重新匹配拼音
            displayCandidateWord(pageCount);//显示候选词
        }
    }
    else
    {
        currentLineEdit->backspace();
    }
}
/*
 *@brief:   切换皮肤
 *@author:  缪庆瑞
 *@date:    2016.12.29
 */
void SoftKeyboard::changeSkinSlot()
{
    if(skinNum == keysAreaStyle.size()-1)
    {
        skinNum = 0;
    }
    else
    {
        skinNum++;
    }
    selectKeyboardStyle(skinNum);
}
/*
 *@brief:   切换数字字母/符号界面
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::changeLetterSymbolSlot()
{
    if(isLetterInput)//当前为字母界面
    {
        if(isENInput)
        {
            this->setSymbolsEN();
        }
        else
        {
            this->setSymbolsCH();
        }
    }
    else  //如果当前为符号界面 返回到数字字母界面
    {
        if(isLetterLower)//小写
        {
            this->setLetterLow();
        }
        else //大写
        {
            this->setLetterUpper();
        }
    }
}
/*
 *@brief:   空格响应槽
 *@author:  缪庆瑞
 *@date:    2016.12.26
 */
void SoftKeyboard::spaceSlot()
{
    if(functionAndCandidateArea->currentWidget() == candidateArea)
    {
        currentLineEdit->insert(candidateWordBtn[0]->text());
        hideCandidateArea();
    }
    else
    {
        currentLineEdit->insert(" ");//插入一个空格
    }
}
/*
 *@brief:   中英文切换
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::changeChEnSlot()
{
    if(isENInput)
    {
        isENInput = false;//切换为中文输入
        chOrEnBtn->setText("CH");
        commaBtn->setText(QString::fromUtf8("，"));
        periodBtn->setText(QString::fromUtf8("。"));
        if(isLetterInput)//字母界面 切换到中文输入时，默认小写
        {
            this->setLetterLow();
        }
        else
        {
            this->setSymbolsCH();
        }
    }
    else
    {
        isENInput = true;//切换为英文输入
        chOrEnBtn->setText("EN");
        commaBtn->setText(",");
        periodBtn->setText(".");
        if(!isLetterInput)//字符界面
        {
            this->setSymbolsEN();
        }
    }
}
/*
 *@brief:   回车响应槽
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::enterSlot()
{
    if(!candidateLetter->text().isEmpty())//候选字母非空，则将字母插入到编辑框里
    {
        currentLineEdit->insert(candidateLetter->text());
        hideCandidateArea();
    }
    else
    {
        if(inputBufferArea->isVisible())//输入缓存区显示时，将缓存的内容通过信号发送出去
        {
            emit sendInputBufferAreaText(inputContentEdit->text());
        }
        clearAndCloseSlot();
    }
}
/*
 *@brief:   清理并关闭键盘
 *@author:  缪庆瑞
 *@date:    2020.05.09
 */
void SoftKeyboard::clearAndCloseSlot()
{
    showInputBufferArea();//显示输入缓存区
    hideCandidateArea();//隐藏候选区
    disconnect(this,0,0,0);//断开键盘所有的信号与槽连接
    this->close();
}
