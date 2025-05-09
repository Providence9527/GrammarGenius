#include "Genius.h"

Genius::Genius(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    task1 = new CodeGenerator();
    task2 = new SLR1();
    connect(task1, SIGNAL(signalBackToMenu1()), this, SLOT(revival()));// 链接任务1信号与槽函数
    connect(task2, SIGNAL(signalBackToMenu2()), this, SLOT(revival()));// 链接任务2信号与槽函数
}

Genius::~Genius()
{}

void Genius::showtask1()
{
    task1->show();
    this->hide();
}
void Genius::showtask2()
{
    task2->show();
    this->hide();
}

void Genius::revival()
{
    this->show();
}

void Genius::showWizard()
{
    QString info;

    info += "软件名称: Grammar Genius (文法天才)\n";
    info += "\n一. 软件简介:\n";
    info += "使用C++语言编写的词法分析器, 能根据某一语言的正则表达式, 生成对应NFA, DFA, 最小化DFA, 词法分析源程序;\n";
    info += "同时能选中词法分析源程序生成的可执行文件(注意: 本软件无法直接生成该可执行文件, 需要用户自行编译)与待编码文件, 最后输出对应单词编码\n";
    info += "\n\n二. 使用方法:\n";
    info += "本软件主要分为以下两个子窗口\n";
    info += "(一) 词法分析子窗口\n";
    info += "视图说明:\n";
    info += "视图1. [input]:   正则表达式输入视图\n";
    info += "功能:\n";
    info += "(1) 正则表达式输入: 支持文本框内输入与左上角菜单项\"open\"打开文本文件输入\n";
    info += "(2) 词法分析源程序命名: 文本框内输入文件名即可完成命名\n";
    info += "(3) 正则表达式保存: 支持左上角菜单项”save”保存文本框正则表达式内容\n";
    info += "视图2. [NFA]: 正则表达式生成的NFA图\n";
    info += "视图3. [DFA]: 正则表达式生成的DFA图\n";
    info += "视图4. [min]: 正则表达式生成的最小化DFA图\n";
    info += "视图5. [run]: 单词编码视图\n";
    info += "功能: \n";
    info += "(1) 浏览文件夹选中词法分析源程序生成的可执行程序: 点击\"open\"\n";
    info += "(2) 输入待编码源程序:支持文本框输入与浏览文件打开\n";
    info += "\n(二) 文法分析子窗口\n";
    info += "视图说明:\n";
    info += "视图1. [Input] : 文法规则输入视图\n";
    info += "功能:\n";
    info += "(1) 文法规则输入: 支持文本框内输入与左上角菜单项\"open\"打开文本文件输入\n";
    info += "(2) 文法规则保存: 支持左上角菜单项”save”保存文本框正则表达式内容\n";
    info += "(3) 展示first集合与follow集合, 是否为文法冲突情况等信息\n";
    info += "视图2. [Map]: 映射表展示视图, 展示文法规则单词-常用ascii字符 1对1 映射表与归约式简写前后的映射\n";
    info += "视图3. [LR(0)DFA]: LR(0)DFA展示视图, 展示邻接矩阵形式的LR(0)DFA图\n";
    info += "视图4. [SLR(1)]: SLR1分析表展示视图, 展示SLR1分析表\n";
    info += "视图5. [Run]: 文法分析视图\n";
    info += "功能:\n";
    info += "(1) 支持文本框输入或者文件浏览输入内容. 若内容为源代码, 则需要另外打开一个对应词法分析可执行程序转成单词编码输入, 如果为单词编码, 直接运行即可进行分析\n";
    info += "(2) 若语句最终被接收, 则输出文法分析过程与语法树; 若不被接收, 则弹出警告信息\n";
    info += "\n三. 警告信息说明\n";
    info += "以下为常见报错符号及其信息:\n";
    info += "E1: Null string received!\n";
    info += "读取转化后, 用来处理的单词编码输入为空串\n";
    info += "E2: Please browse to open a file or directly enter text in the text box\n";
    info += "[Run]视图文本输入框为空\n";
    info += "E3: Please select the input mode (lex file/source file)\n";
    info += "[Run]视图没有选择输入模式\n";
    info += "E4: Please switch to SOURCE mode!\n";
    info += "[Run]视图选择可执行程序时没有切换到SOURCE模式\n";
    info += "E5: Please select the path for the EXE\n";
    info += "[Run]视图文本框输入的为源代码时, 没有请选择可执行程序的路径\n";
    info += "E6: Please generate the SLR1 parsing table before performing syntax analysis.\n";
    info += "[Run]视图, 文法分析前没有生成SLR1就来进行语法分析\n";
    info += "E7: The statement you entered is not grammatically correct and therefore cannot be accepted.\n";
    info += "[Run]视图, 输入不符合语法, 即不接受\n";
    info += "E8: Please select the PROPER input mode (lex file/source file)\n";
    info += "[Run]视图选择的输入模式与文本框文本不匹配\n";

    QDialog dialog;
    dialog.setWindowTitle("使用说明");
    dialog.setFixedSize(800, 600); // 设置QDialog的大小为400x300像素

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    QTextEdit* textEdit = new QTextEdit(&dialog);
    textEdit->setPlainText(info);
    textEdit->setReadOnly(true);
    layout->addWidget(textEdit);

    QPushButton* okButton = new QPushButton("确定", &dialog);
    layout->addWidget(okButton);

    QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}


