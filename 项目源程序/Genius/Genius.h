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
    CodeGenerator* task1;//任务1窗口
    SLR1* task2;// 任务2窗口


private slots:
    void  showtask1();// 展现任务1窗口的函数
    void showtask2();// 展现任务2窗口的函数
    void revival();// 重新显示主菜单
    void showWizard();// 显示使用说明的函数

};
