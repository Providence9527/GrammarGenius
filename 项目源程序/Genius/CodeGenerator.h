#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_CodeGenerator.h"
#include <QTextBlock>
#include <QtCore/QTextStream>
#include<QFileDialog>
#include<QErrorMessage>
#include<QMessageBox>



class CodeGenerator : public QMainWindow
{
    Q_OBJECT

public:
    CodeGenerator(QWidget *parent = nullptr);
    ~CodeGenerator();

private:
    Ui::CodeGeneratorClass ui;
    void init();
    
signals:
    void signalBackToMenu1(); //向主窗口发送返回信息的函数

private slots://相应功能槽函数
    void test();
    void openfile();
    void savefile();
    void choexe();
    void chosrccode();
    void wordsEncode();
    void backToMenu1();// 返回主菜单函数

};
