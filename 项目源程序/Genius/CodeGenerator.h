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
    void signalBackToMenu1(); //�������ڷ��ͷ�����Ϣ�ĺ���

private slots://��Ӧ���ܲۺ���
    void test();
    void openfile();
    void savefile();
    void choexe();
    void chosrccode();
    void wordsEncode();
    void backToMenu1();// �������˵�����

};
