/*
 *@file:   softkeyboard.cpp
 *@author: 缪庆瑞
 *@date:   2016.12.25
 *@brief:  软键盘部件，实现中英文输入
 */
#include "softkeyboard.h"
#include <QBoxLayout>
#include <QDebug>
#include <QDateTime>
#define PINYINFILEPATH  "./ChinesePinyin"
#define FONTFAMILY "黑体"  //字体族
#define FONTSIZE 16      //字体大小

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
    this->initTextBufferArea();
    this->initFunctionAndCandidateArea();
    this->initKeyBoardArea();
    this->setLetterLow();//默认显示字母界面 小写
    this->selectKeyboardStyle(0);//选择皮肤
    this->setMoveEnabled();
    this->setTextBufferAreaVisibled(false);
    skinNum = 0;
    isENInput = true;//初始化为英文输入
    isLetterInput = true;//初始化为字母界面
    isLetterLower = true;//小写字母
    //整体垂直布局
    QVBoxLayout *globalVLayout = new QVBoxLayout(this);
    globalVLayout->setMargin(2);
    globalVLayout->setSpacing(0);
    globalVLayout->addWidget(textBufferArea,1);
    globalVLayout->addWidget(functionAndCandidateArea,1);
    globalVLayout->addWidget(keyBoardArea,7);

    readDictionary();//读拼音字典
}

SoftKeyboard::~SoftKeyboard()
{

}
/*
 *@author:  缪庆瑞
 *@date:    2016.12.29
 *@brief:   读拼音字典，将汉字与对应拼音存放到hash表中,也可以是QMap中，但不考虑排列顺序时，hash更快
 */
void SoftKeyboard::readDictionary()
{
    //通过打印时间测试读拼音文件的效率 8G内存windows测试大约用1/10s
    qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd HH:m:s:z");
    QFile pinyinFile(PINYINFILEPATH);
    if(!pinyinFile.open(QIODevice::ReadOnly))//拼音文件打开失败提示
    {
        QMessageBox::critical(this,tr("Open File Failed"),tr("无法打开拼音文件。。。"));
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
            lineText = QString(tr(pinyinFile.readLine()));
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
 *@author:  缪庆瑞
 *@date:    2017.1.12
 *@brief:   初始化样式表，即用于界面皮肤选择
 */
void SoftKeyboard::initStyleSheet()
{
    /*皮肤1:雅黑*/
    //按键区域样式
    keyBoardAreaStyle.append(
                "background-color:#1E1E1E;""color:#E6E6E6;");
    //功能和候选区区域样式 目前与按键区域样式一致
    functionAndCandidateAreaStyle.append(
                "background-color:#1E1E1E;color:#E6E6E6;");
    //候选字母样式
    candidateLetterStyle.append(
                "background-color:#4E4E4E;");
    //普通按键的样式
    commonKeyStyle.append(
                "QPushButton{background-color:#4E4E4E;border-radius:4px;}"
                "QPushButton:focus{outline:none;}");
    //特殊按键的样式
    specialKeyStyle.append(
                "QPushButton{background-color:#2D2D2D;border-radius:2px;}"
                "QPushButton:focus{outline:none;}");
    /*皮肤2:简白*/
    keyBoardAreaStyle.append(
                "background-color:#D8D8D8;color:black;");
    functionAndCandidateAreaStyle.append(
                "background-color:#D8D8D8;color:black;");
    candidateLetterStyle.append(
                "background-color:white;");
    commonKeyStyle.append(
                "QPushButton{background-color:#FFFFFF;border-radius:6px;}"
                "QPushButton:focus{outline:none;}");
    specialKeyStyle.append(
                "QPushButton{background-color:#EBEBEB;border-radius:6px;}"
                "QPushButton:focus{outline:none;}");
    /*皮肤3:魅紫*/
    keyBoardAreaStyle.append(
                "background-color:#190724;color:#68CBF2;");
    functionAndCandidateAreaStyle.append(
                "background-color:#190724;color:#68CBF2;");
    candidateLetterStyle.append(
                "background-color:#272A5E;");
    commonKeyStyle.append(
                "QPushButton{background-color:#272A5E;border-radius:4px;}"
                "QPushButton:focus{outline:none;}");
    specialKeyStyle.append(
                "QPushButton{background-color:#241D48;border-radius:2px;}"
                "QPushButton:focus{outline:none;}");
    /*皮肤4:*/
    keyBoardAreaStyle.append(
                "background-color:#F8859F;color:black;");
    functionAndCandidateAreaStyle.append(
                "background-color:#F8859F;color:black;");
    candidateLetterStyle.append(
                "background-color:#F6CED6;");
    commonKeyStyle.append(
                "QPushButton{background-color:#F6CED6;border-radius:12px;}"
                "QPushButton:pressed{background-color:orange;border-radius:12px;}"
                "QPushButton:focus{outline:none;}");
    specialKeyStyle.append(
                "QPushButton{background-color:#FFA3B8;border-radius:12px;}"
                "QPushButton:pressed{background-color:orange;border-radius:12px;}"
                "QPushButton:focus{outline:none;}");

}

/*
 *@author:  缪庆瑞
 *@date:    2016.12.25
 *@brief:   初始化软键盘的数字，字母或字符按键
 */
void SoftKeyboard::initNumberLetterBtn()
{
    QFont font(tr(FONTFAMILY),FONTSIZE);
    //以下36个按键，仅作为普通输入，无其他功能，所以连接一个槽函数
    for(int i=0;i<36;i++)//为10个数字，26个字母按键申请空间,连接信号与槽
    {
        numberLetterBtn[i] = new QPushButton();
        numberLetterBtn[i]->setFont(font);
        numberLetterBtn[i]->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
        connect(numberLetterBtn[i],SIGNAL(clicked()),this,SLOT(numberLetterBtnSlot()));
    }
}
/*
 *@author:  缪庆瑞
 *@date:    2016.12.26
 *@brief:   初始化软键盘的特殊功能按键
 */
void SoftKeyboard::initSpecialBtn()
{
    QFont font(tr(FONTFAMILY),FONTSIZE);
    upperOrLowerBtn = new QPushButton();
    upperOrLowerBtn->setFont(font);
    upperOrLowerBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    upperOrLowerBtn->setText(tr("A/a"));
    connect(upperOrLowerBtn,SIGNAL(clicked()),this,SLOT(changeUpperLowerSlot()));

    deleteBtn = new QPushButton();
    deleteBtn->setFont(font);
    deleteBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    deleteBtn->setText("del");
    //删除按钮按下即会删除
    connect(deleteBtn,SIGNAL(pressed()),this,SLOT(deleteTextSlot()));
    connect(deleteBtn,SIGNAL(released()),this,SLOT(closeDelTimerSlot()));//焦点离开按钮也会触发该信号
    delTimer = new QTimer(this);
    connect(delTimer,SIGNAL(timeout()),this,SLOT(deleteTextSlot()));

    skinBtn = new QPushButton();
    skinBtn->setFont(font);
    skinBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    skinBtn->setText(tr("  皮肤  "));
    connect(skinBtn,SIGNAL(clicked()),this,SLOT(changeSkinSlot()));

    letterOrSymbolBtn = new QPushButton();
    letterOrSymbolBtn->setFont(font);
    letterOrSymbolBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    letterOrSymbolBtn->setText(tr("ab/符"));
    connect(letterOrSymbolBtn,SIGNAL(clicked()),this,SLOT(changeLetterSymbolSlot()));

    commaBtn = new QPushButton();
    commaBtn->setMinimumSize(36,16);//设置最小大小，主要是为了清掉布局的默认最佳大小
    commaBtn->setFont(font);
    commaBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    commaBtn->setText(",");
    connect(commaBtn,SIGNAL(clicked()),this,SLOT(numberLetterBtnSlot()));

    spaceBtn = new QPushButton();
    spaceBtn->setText(" ");
    spaceBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    connect(spaceBtn,SIGNAL(clicked()),this,SLOT(spaceSlot()));

    periodBtn = new QPushButton();
    periodBtn->setMinimumSize(36,16);
    periodBtn->setFont(font);
    periodBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    periodBtn->setText(".");
    connect(periodBtn,SIGNAL(clicked()),this,SLOT(numberLetterBtnSlot()));

    chOrEnBtn = new QPushButton();
    chOrEnBtn->setFont(font);
    chOrEnBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    chOrEnBtn->setText(tr("  英  "));
    connect(chOrEnBtn,SIGNAL(clicked()),this,SLOT(changeChEnSlot()));

    enterBtn = new QPushButton();
    enterBtn->setFont(font);
    enterBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    enterBtn->setText("  Enter  ");
    connect(enterBtn,SIGNAL(clicked()),this,SLOT(enterSlot()));
}
/*
 *@author:  缪庆瑞
 *@date:    2019.7.5
 *@brief:   初始化键盘输入缓存区，临时存放显示输入的内容
 */
void SoftKeyboard::initTextBufferArea()
{
    QFont font(tr(FONTFAMILY),FONTSIZE-2);
    textBufferArea = new QWidget();
    QHBoxLayout *hBoxLayout = new QHBoxLayout(textBufferArea);
    hBoxLayout->setContentsMargins(5,9,5,20);
    titleLabel = new QLabel();
    titleLabel->setFont(font);
    titleLabel->setText(tr("输入测试:"));
    //键盘输入内容显示部件
    lineEdit = new QLineEdit();
    currentLineEdit = lineEdit;
    lineEdit->setFont(font);
    lineEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

    hBoxLayout->addWidget(titleLabel);
    hBoxLayout->addWidget(lineEdit);
}

/*
 *@author:  缪庆瑞
 *@date:    2019.7.3
 *@brief:   初始化功能和候选词区域,该区域使用栈部件,同一时刻只显示一个区域
 */
void SoftKeyboard::initFunctionAndCandidateArea()
{
    /*****************功能区*****************/
    QFont functionAreaFont(tr(FONTFAMILY),FONTSIZE+2);
    functionArea = new QWidget();
    QHBoxLayout *hBoxLayout = new QHBoxLayout(functionArea);
    hBoxLayout->setContentsMargins(8,0,8,0);
    introduceLabel = new QLabel();
    introduceLabel->setAlignment(Qt::AlignCenter);
    introduceLabel->setFont(functionAreaFont);
    introduceLabel->setText(tr("欢迎使用中文输入软键盘"));
    hBoxLayout->addWidget(introduceLabel);
    /*****************候选区*****************/
    QFont candidateAreaFont(tr(FONTFAMILY),FONTSIZE-2);
    candidateArea = new QWidget();
    QVBoxLayout *vBoxLayout = new QVBoxLayout(candidateArea);
    vBoxLayout->setMargin(0);
    vBoxLayout->setSpacing(0);
    //候选字母显示框
    candidateLetter = new QLineEdit();
    candidateLetter->setFrame(false);
    connect(candidateLetter,SIGNAL(textChanged(QString)),this,SLOT(candidateLetterChangedSlot(QString)));
    candidateLetter->setFont(candidateAreaFont);
    candidateLetter->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);//水平方向固定大小
    //候选词、翻页按钮显示区域
    candidateWordArea = new QWidget();
    candidateWordArea->setFixedHeight(QFontMetrics(candidateAreaFont).height()+2);//由所设字体大小固定该区域高度
    QHBoxLayout *hBoxLayout2 = new QHBoxLayout(candidateWordArea);
    hBoxLayout2->setMargin(0);
    for(int i=0;i<CANDIDATEWORDNUM;i++)
    {
        candidateWordBtn[i] = new QPushButton();
        candidateWordBtn[i]->setFlat(true);
        candidateWordBtn[i]->setFont(candidateAreaFont);
        candidateWordBtn[i]->setStyleSheet("QPushButton:focus{outline:none;}");//去除虚线框
        candidateWordBtn[i]->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);   
        connect(candidateWordBtn[i],SIGNAL(clicked()),this,SLOT(candidateWordBtnSlot()));
        hBoxLayout2->addWidget(candidateWordBtn[i]);
    }
    prePageBtn = new QPushButton();//上一页
    prePageBtn->setFlat(true);
    prePageBtn->setFont(candidateAreaFont);
    prePageBtn->setText("<");
    prePageBtn->setStyleSheet("QPushButton:focus{outline:none;}");
    prePageBtn->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred);
    connect(prePageBtn,SIGNAL(clicked()),this,SLOT(candidateWordPrePageSlot()));
    nextPageBtn = new QPushButton();//下一页
    nextPageBtn->setFlat(true);
    nextPageBtn->setFont(candidateAreaFont);
    nextPageBtn->setText(">");
    nextPageBtn->setStyleSheet("QPushButton:focus{outline:none;}");
    nextPageBtn->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred);
    connect(nextPageBtn,SIGNAL(clicked()),this,SLOT(candidateWordNextPageSlot()));
    hBoxLayout2->addWidget(prePageBtn);
    hBoxLayout2->addWidget(nextPageBtn);

    vBoxLayout->addWidget(candidateLetter);
    vBoxLayout->addWidget(candidateWordArea);
    /***************栈部件存放功能区和候选区******************/
    functionAndCandidateArea = new QStackedWidget();
    functionAndCandidateArea->addWidget(functionArea);
    functionAndCandidateArea->addWidget(candidateArea);
}
/*
 *@author:  缪庆瑞
 *@date:    2016.12.25
 *@brief:   初始化键盘按键区域
 */
void SoftKeyboard::initKeyBoardArea()
{
    initNumberLetterBtn();//初始化10个数字，26个字母或字符按键
    initSpecialBtn();//初始化特殊按键
    keyBoardArea = new QWidget();
    //keyBoardArea->setStyleSheet("background-color:black;");
    QHBoxLayout *firstRowHLayout = new QHBoxLayout();
    for(int i=0;i<10;i++)//布局第一排按键 10个数字
    {
        firstRowHLayout->addWidget(numberLetterBtn[i]);
    }
    QHBoxLayout *secondRowHLayout = new QHBoxLayout();
    for(int i=10;i<20;i++)//布局第二排按键
    {
        secondRowHLayout->addWidget(numberLetterBtn[i]);
    }
    QHBoxLayout *thirdRowHLayout = new QHBoxLayout();
    thirdRowHLayout->setContentsMargins(20,0,20,0);
    for(int i=20;i<29;i++)//布局第三排按键
    {
        thirdRowHLayout->addWidget(numberLetterBtn[i]);
    }
    QHBoxLayout *fourthRowHLayout = new QHBoxLayout();
    fourthRowHLayout->addWidget(upperOrLowerBtn);//大小写切换按键
    for(int i =29;i<36;i++)//布局第四排按键
    {
        fourthRowHLayout->addWidget(numberLetterBtn[i]);
    }
    fourthRowHLayout->addWidget(deleteBtn);//删除按键
    QHBoxLayout *fifthRowHLayout = new QHBoxLayout();
    //布局第五排按键，基本为特殊功能按键
    fifthRowHLayout->addWidget(skinBtn,2);
    fifthRowHLayout->addWidget(letterOrSymbolBtn,1);
    fifthRowHLayout->addWidget(commaBtn,1);
    fifthRowHLayout->addWidget(spaceBtn,4);
    fifthRowHLayout->addWidget(periodBtn,1);
    fifthRowHLayout->addWidget(chOrEnBtn,1);
    fifthRowHLayout->addWidget(enterBtn,2);

    QVBoxLayout *vBoxlayout = new QVBoxLayout(keyBoardArea);
    vBoxlayout->setContentsMargins(8,2,8,8);
    vBoxlayout->addLayout(firstRowHLayout);
    vBoxlayout->addLayout(secondRowHLayout);
    vBoxlayout->addLayout(thirdRowHLayout);
    vBoxlayout->addLayout(fourthRowHLayout);
    vBoxlayout->addLayout(fifthRowHLayout);
}
/*
 *@author:  缪庆瑞
 *@date:    2016.12.25
 *@brief:   设置小写字母显示
 */
void SoftKeyboard::setLetterLow()
{
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
 *@author:  缪庆瑞
 *@date:    2016.12.25
 *@brief:   设置大写字母显示
 */
void SoftKeyboard::setLetterUpper()
{
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
 *@author:  缪庆瑞
 *@date:    2016.12.31
 *@brief:   设置符号显示，包含所以可输入符号(英文状态)
 */
void SoftKeyboard::setSymbolsEN()
{
    QStringList symbolsENList;
    symbolsENList<<"!"<<"@"<<"#"<<"$"<<"%"<<"^"<<"&&"<<"*"<<"("<<")"
               <<"["<<"]"<<"{"<<"}"<<"<"<<">"<<"+"<<"-"<<"_"<<"="
               <<"`"<<"~"<<"|"<<"\\"<<"'"<<"\""<<":"<<";"<<"?"
               <<"/"<<"www."<<".com"<<".cn"<<".net"<<tr("→")<<tr("←");
    for(int i=0;i<36;i++)
    {
        numberLetterBtn[i]->setText(symbolsENList.at(i));
    }
}
/*
 *@author:  缪庆瑞
 *@date:    2016.12.31
 *@brief:   设置符号显示，包含所有可输入符号(中文状态)
 */
void SoftKeyboard::setSymbolsCH()
{
    QStringList symbolsCHList;
    //中文状态下的字符用tr编码转换，实现Qt4的兼容，Qt4使用QTextCodec实现utf-8编码中文
    symbolsCHList<<tr("！")<<tr("@")<<tr("#")<<tr("￥")<<tr("%")<<tr("……")<<tr("&&")<<tr("*")<<tr("（")<<tr("）")
                <<tr("【")<<tr("】")<<tr("{")<<tr("}")<<tr("《")<<tr("》")<<tr("+")<<tr("-")<<tr("—")<<tr("=")
                <<tr("·")<<tr("~")<<tr("|")<<tr("、")<<tr("’")<<tr("‘")<<tr("“")<<tr("”")<<tr("：")
                <<tr("；")<<tr("？")<<tr("♀")<<tr("℃")<<tr("★")<<tr("↑")<<tr("↓");
    for(int i=0;i<36;i++)
    {
        numberLetterBtn[i]->setText(symbolsCHList.at(i));
    }
}
/*
 *@author:  缪庆瑞
 *@date:    2017.2.7
 *@brief:   拆分拼音词组，拼音字典文件词组用'分割，如"ai'qing"该函数的功能便是去掉'，将简拼、
 *              全拼存放到哈希表中
 *@param1:   phrase:要处理的拼音词组
 *@param2:  chinese:拼音对应的汉字
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
 *@author:  缪庆瑞
 *@date:    2017.1.1
 *@brief:   根据输入的拼音匹配中文
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
 *@author:  缪庆瑞
 *@date:    2017.1.1
 *@brief:   显示指定页的候选词 page从1开始
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
 *@author:  缪庆瑞
 *@date:    2017.1.4
 *@brief:   隐藏中文输入的候选区域
 */
void SoftKeyboard::hideCandidateArea()
{
    candidateLetter->clear();//清空候选字母
    functionAndCandidateArea->setCurrentWidget(functionArea);//显示功能区
}
/*
 *@author:  缪庆瑞
 *@date:    2017.1.12
 *@brief:   选择键盘样式，所有按键及区域背景的样式在此设置
 *@param:   num：确定用哪一套样式（皮肤）
 */
void SoftKeyboard::selectKeyboardStyle(int num)
{
    if(num>=keyBoardAreaStyle.size())
    {
        qDebug()<<"该索引皮肤不存在,已采用默认皮肤设置";
        return;//避免超出list长度，程序异常
    }
    //用于按键区域的样式
    QString tmpStyle = keyBoardAreaStyle.at(num);
    keyBoardArea->setStyleSheet(tmpStyle);
    //用于功能和候选区区域的样式
    tmpStyle = functionAndCandidateAreaStyle.at(num);
    functionAndCandidateArea->setStyleSheet(tmpStyle);
    //用于候选字母显示框的样式
    tmpStyle = candidateLetterStyle.at(num);
    candidateLetter->setStyleSheet(tmpStyle);
    //用于普通按键的样式
    tmpStyle = commonKeyStyle.at(num);
    for(int i=0;i<36;i++)//36个普通输入按键样式
    {
        numberLetterBtn[i]->setStyleSheet(tmpStyle);
    }
    letterOrSymbolBtn->setStyleSheet(tmpStyle);//几个特殊按键同样设置成普通样式
    commaBtn->setStyleSheet(tmpStyle);
    spaceBtn->setStyleSheet(tmpStyle);
    periodBtn->setStyleSheet(tmpStyle);
    chOrEnBtn->setStyleSheet(tmpStyle);
    //用于特殊按键的样式
    tmpStyle = specialKeyStyle.at(num);
    upperOrLowerBtn->setStyleSheet(tmpStyle);
    deleteBtn->setStyleSheet(tmpStyle);
    skinBtn->setStyleSheet(tmpStyle);
    enterBtn->setStyleSheet(tmpStyle);
}
/*
 *@author:  缪庆瑞
 *@date:    2019.7.5
 *@brief:   设置无边框窗口是否可以移动
 */
void SoftKeyboard::setMoveEnabled(bool moveEnabled)
{
    isMoveEnabled = moveEnabled;
}
/*
 *@author:  缪庆瑞
 *@date:    2019.7.5
 *@brief:   设置输入缓存区是否显示
 *@param:   isVisibled:是否显示
 */
void SoftKeyboard::setTextBufferAreaVisibled(bool isVisibled)
{
    textBufferArea->setVisible(isVisibled);
}
/*
 *@author:  缪庆瑞
 *@date:    2019.7.5
 *@brief:   设置默认输入缓存区的label显示
 */
void SoftKeyboard::setTitleLabelText(const QString &titleLabelText)
{
    titleLabel->setText(titleLabelText);
}
/*
 *@author:  缪庆瑞
 *@date:    2019.7.5
 *@brief:   设置键盘当前聚焦的输入框的显示文本
 */
void SoftKeyboard::setCurrentLineEditText(const QString &currentEidtText)
{
    currentLineEdit->setText(currentEidtText);
}
/*
 *@author:  缪庆瑞
 *@date:    2019.7.5
 *@brief:   设置键盘当前聚焦的输入框
 *@param:   currentEdit:当前聚焦的输入框,如果为NULL则设置为lineEdit
 */
void SoftKeyboard::setCurrentLineEdit(QLineEdit *currentEdit)
{
    if(currentEdit == NULL)
    {
        currentLineEdit = lineEdit;
    }
    else
    {
        currentLineEdit = currentEdit;
    }
}
/*
 *@author:  缪庆瑞
 *@date:    2019.7.5
 *@brief:   鼠标按下事件处理
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
 *@author:  缪庆瑞
 *@date:    2019.7.5
 *@brief:   鼠标移动事件处理
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
 *@author:  缪庆瑞
 *@date:    2019.7.5
 *@brief:   鼠标释放事件处理
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
 *@author:  缪庆瑞
 *@date:    2016.12.25
 *@brief:   字母(符号)按键被点击的响应槽
 */
void SoftKeyboard::numberLetterBtnSlot()
{
    QPushButton *clickedBtn = qobject_cast<QPushButton *>(sender());//获取信号发送者的对象
    if(isENInput||!isLetterInput)//英文输入模式或者字符输入界面
    {
        if(clickedBtn->text()=="&&")//因为可显示控件把&符号当成快捷键标志，一个不显示，所以这个要做下特别处理
        {
            currentLineEdit->insert("&");
            currentLineEdit->setFocus();
        }
        else
        {
            currentLineEdit->insert(clickedBtn->text());//文本输入框插入字母或符号
            currentLineEdit->setFocus();//为编辑框设置焦点，以使输入时在编辑框有闪烁光标提示
        }
    }
    else  //中文输入模式 键入的字母放在第二部分输入显示区域的候选字母按钮上
    {
        if(clickedBtn->text()==tr("，")||clickedBtn->text()==tr("。"))//，。特殊情况
        {
            currentLineEdit->insert(clickedBtn->text());//文本输入框插入字母或符号
            currentLineEdit->setFocus();//为编辑框设置焦点，以使输入时在编辑框有闪烁光标提示
        }
        else
        {
            functionAndCandidateArea->setCurrentWidget(candidateArea);
            candidateLetter->insert(clickedBtn->text());//候选字母输入框插入字母
            candidateLetter->setFocus();
            this->matchChinese(candidateLetter->text());//匹配中文
            this->displayCandidateWord(pageCount);//显示候选词，默认第一页
            //prePageBtn->setEnabled(false);//向前翻页不可点击
        }
    }
}
/*
 *@author:  缪庆瑞
 *@date:    2017.1.2
 *@brief:   中文输入时候选字母区域根据内容改变文本框的大小
 *@param:   text:即改变后的文本内容
 */
void SoftKeyboard::candidateLetterChangedSlot(QString text)
{
    //根据输入的内容自动改变文本区域大小
    int width = candidateLetter->fontMetrics().width(text)+6;
    candidateLetter->setFixedWidth(width);
}
/*
 *@author:  缪庆瑞
 *@date:    2016.12.25
 *@brief:   候选词被点击的响应槽
 */
void SoftKeyboard::candidateWordBtnSlot()
{
    QPushButton *clickedBtn = qobject_cast<QPushButton *>(sender());//获取信号发送者的对象
    currentLineEdit->insert(clickedBtn->text());
    currentLineEdit->setFocus();
    hideCandidateArea();//隐藏中文候选区域
}
/*
 *@author:  缪庆瑞
 *@date:    2016.12.29
 *@brief:   候选词向前翻页
 */
void SoftKeyboard::candidateWordPrePageSlot()
{
    pageCount--;
    this->displayCandidateWord(pageCount);
    //避免点击向前翻页按钮时取消使能，焦点自动跑到下一个按钮上。
    candidateLetter->setFocus();
}
/*
 *@author:  缪庆瑞
 *@date:    2016.12.29
 *@brief:   候选词向后翻页
 */
void SoftKeyboard::candidateWordNextPageSlot()
{
    pageCount++;
    this->displayCandidateWord(pageCount);
    //避免点击向后翻页按钮时取消使能，焦点自动跑到下一个按钮上
    candidateLetter->setFocus();
}
/*
 *@author:  缪庆瑞
 *@date:    2016.12.26
 *@brief:   大小写切换 也可以切换数字字母与字符界面
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
        isLetterLower = !isLetterLower;//将状态取反
    }
    else  //当前为数字字符界面,则返回之前的大小写字母界面
    {
        isLetterInput = true;
        if(isLetterLower)
        {
            this->setLetterLow();
        }
        else
        {
            this->setLetterUpper();
        }
    }
    currentLineEdit->setFocus();//将焦点返回到行编辑栏，避免某些按钮有焦点时会自动设置样式
}
/*
 *@author:  缪庆瑞
 *@date:    2016.12.26
 *@brief:   删除输入内容
 */
void SoftKeyboard::deleteTextSlot()
{
    delTimer->start(150);//开启150ms的定时器 连续删除
    if(functionAndCandidateArea->currentWidget() == candidateArea)
    {
        //删除时不可以转移焦点，这样会使del按钮的释放事件得不到响应，无法关闭定时器
        //candidateLetter->setFocus();
        candidateLetter->backspace();//删除选中文本或光标前的一个字符，默认光标在最后
        if(candidateLetter->text().isEmpty())//删完了
        {
            hideCandidateArea();
            delTimer->stop();
        }
        else
        {
            matchChinese(candidateLetter->text());//重新匹配拼音
            displayCandidateWord(pageCount);//显示候选词
        }
    }
    else
    {
        //currentLineEdit->setFocus();
        currentLineEdit->backspace();
        if(currentLineEdit->text().isEmpty())
        {
            delTimer->stop();//删完后主动停止定时器
        }
    }
}
/*
 *@author:  缪庆瑞
 *@date:    2017.1.2
 *@brief:   关闭连续删除的定时器,并设置焦点
 */
void SoftKeyboard::closeDelTimerSlot()
{
    delTimer->stop();
    if(functionAndCandidateArea->currentWidget() == candidateArea)
    {
        candidateLetter->setFocus();//候选字母设置焦点
    }
    else
    {
        currentLineEdit->setFocus();//编辑框设置焦点
    }
}
/*
 *@author:  缪庆瑞
 *@date:    2016.12.29
 *@brief:   切换皮肤
 */
void SoftKeyboard::changeSkinSlot()
{
    if(skinNum == keyBoardAreaStyle.size()-1)
    {
        skinNum = 0;
    }
    else
    {
        skinNum++;
    }
    selectKeyboardStyle(skinNum);
    currentLineEdit->setFocus();//将焦点返回到行编辑栏，避免某些按钮有焦点时会自动设置样式
}
/*
 *@author:  缪庆瑞
 *@date:    2016.12.26
 *@brief:   切换数字字母/符号界面 中英文输入下的部分符号不同
 */
void SoftKeyboard::changeLetterSymbolSlot()
{
    if(isLetterInput)//当前为字母界面
    {
        isLetterInput = false;//非字母界面
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
        isLetterInput = true;//字母界面
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
 *@author:  缪庆瑞
 *@date:    2016.12.26
 *@brief:   空格响应槽
 */
void SoftKeyboard::spaceSlot()
{
    if(functionAndCandidateArea->currentWidget() == candidateArea)
    {
        currentLineEdit->insert(candidateWordBtn[0]->text());
        currentLineEdit->setFocus();
        hideCandidateArea();
    }
    else
    {
        currentLineEdit->insert(" ");//插入一个空格
        currentLineEdit->setFocus();
    }
}
/*
 *@author:  缪庆瑞
 *@date:    2016.12.26
 *@brief:   中英文切换
 */
void SoftKeyboard::changeChEnSlot()
{
    if(isENInput)
    {
        isENInput = false;//切换为中文输入
        chOrEnBtn->setText(tr("  中  "));
        commaBtn->setText(tr("，"));
        periodBtn->setText(tr("。"));
        if(isLetterInput)//字母界面 切换到中文输入时，默认小写
        {
            isLetterLower = true;
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
        chOrEnBtn->setText(tr("  英  "));
        commaBtn->setText(",");
        periodBtn->setText(".");
        if(!isLetterInput)//字符界面
        {
            this->setSymbolsEN();
        }
    }
    currentLineEdit->setFocus();//将焦点返回到行编辑栏，避免某些按钮有焦点时会自动设置样式
}
/*
 *@author:  缪庆瑞
 *@date:    2016.12.26
 *@brief:   回车响应槽
 */
void SoftKeyboard::enterSlot()
{
    if(!candidateLetter->text().isEmpty())//候选字母非空，则将字母插入到编辑框里
    {
        currentLineEdit->insert(candidateLetter->text());
        currentLineEdit->setFocus();
        hideCandidateArea();
    }
    else
    {
        quitSlot();
    }
}
/*
 *@author:  缪庆瑞
 *@date:    2017.1.2
 *@brief:   软键盘退出
 */
void SoftKeyboard::quitSlot()
{
    emit sendText(currentLineEdit->text());
    //退出时清空默认缓存区
    lineEdit->clear();
    hideCandidateArea();
    this->close();
}
