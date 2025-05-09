#pragma once
#pragma execution_character_set("UTF-8")
#include <QtWidgets/QMainWindow>
#include "ui_Genius.h"
#include"CodeGenerator.h"
#include"SLR1.h"

class Genius : public QMainWindow
{
    Q_OBJECT

public:
    Genius(QWidget *parent = nullptr);
    ~Genius();

private:
    Ui::GeniusClass ui; 
    CodeGenerator* task1;//����1����
    SLR1* task2;// ����2����


private slots:
    void  showtask1();// չ������1���ڵĺ���
    void showtask2();// չ������2���ڵĺ���
    void revival();// ������ʾ���˵�
    void showWizard();// ��ʾʹ��˵���ĺ���

};
