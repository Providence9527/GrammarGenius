#include "Genius.h"

Genius::Genius(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    task1 = new CodeGenerator();
    task2 = new SLR1();
    connect(task1, SIGNAL(signalBackToMenu1()), this, SLOT(revival()));// ��������1�ź���ۺ���
    connect(task2, SIGNAL(signalBackToMenu2()), this, SLOT(revival()));// ��������2�ź���ۺ���
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

    info += "�������: Grammar Genius (�ķ����)\n";
    info += "\nһ. ������:\n";
    info += "ʹ��C++���Ա�д�Ĵʷ�������, �ܸ���ĳһ���Ե�������ʽ, ���ɶ�ӦNFA, DFA, ��С��DFA, �ʷ�����Դ����;\n";
    info += "ͬʱ��ѡ�дʷ�����Դ�������ɵĿ�ִ���ļ�(ע��: ������޷�ֱ�����ɸÿ�ִ���ļ�, ��Ҫ�û����б���)��������ļ�, ��������Ӧ���ʱ���\n";
    info += "\n\n��. ʹ�÷���:\n";
    info += "�������Ҫ��Ϊ���������Ӵ���\n";
    info += "(һ) �ʷ������Ӵ���\n";
    info += "��ͼ˵��:\n";
    info += "��ͼ1. [input]:   ������ʽ������ͼ\n";
    info += "����:\n";
    info += "(1) ������ʽ����: ֧���ı��������������Ͻǲ˵���\"open\"���ı��ļ�����\n";
    info += "(2) �ʷ�����Դ��������: �ı����������ļ��������������\n";
    info += "(3) ������ʽ����: ֧�����Ͻǲ˵��save�������ı���������ʽ����\n";
    info += "��ͼ2. [NFA]: ������ʽ���ɵ�NFAͼ\n";
    info += "��ͼ3. [DFA]: ������ʽ���ɵ�DFAͼ\n";
    info += "��ͼ4. [min]: ������ʽ���ɵ���С��DFAͼ\n";
    info += "��ͼ5. [run]: ���ʱ�����ͼ\n";
    info += "����: \n";
    info += "(1) ����ļ���ѡ�дʷ�����Դ�������ɵĿ�ִ�г���: ���\"open\"\n";
    info += "(2) ���������Դ����:֧���ı�������������ļ���\n";
    info += "\n(��) �ķ������Ӵ���\n";
    info += "��ͼ˵��:\n";
    info += "��ͼ1. [Input] : �ķ�����������ͼ\n";
    info += "����:\n";
    info += "(1) �ķ���������: ֧���ı��������������Ͻǲ˵���\"open\"���ı��ļ�����\n";
    info += "(2) �ķ����򱣴�: ֧�����Ͻǲ˵��save�������ı���������ʽ����\n";
    info += "(3) չʾfirst������follow����, �Ƿ�Ϊ�ķ���ͻ�������Ϣ\n";
    info += "��ͼ2. [Map]: ӳ���չʾ��ͼ, չʾ�ķ����򵥴�-����ascii�ַ� 1��1 ӳ������Լʽ��дǰ���ӳ��\n";
    info += "��ͼ3. [LR(0)DFA]: LR(0)DFAչʾ��ͼ, չʾ�ڽӾ�����ʽ��LR(0)DFAͼ\n";
    info += "��ͼ4. [SLR(1)]: SLR1������չʾ��ͼ, չʾSLR1������\n";
    info += "��ͼ5. [Run]: �ķ�������ͼ\n";
    info += "����:\n";
    info += "(1) ֧���ı�����������ļ������������. ������ΪԴ����, ����Ҫ�����һ����Ӧ�ʷ�������ִ�г���ת�ɵ��ʱ�������, ���Ϊ���ʱ���, ֱ�����м��ɽ��з���\n";
    info += "(2) ��������ձ�����, ������ķ������������﷨��; ����������, �򵯳�������Ϣ\n";
    info += "\n��. ������Ϣ˵��\n";
    info += "����Ϊ����������ż�����Ϣ:\n";
    info += "E1: Null string received!\n";
    info += "��ȡת����, ��������ĵ��ʱ�������Ϊ�մ�\n";
    info += "E2: Please browse to open a file or directly enter text in the text box\n";
    info += "[Run]��ͼ�ı������Ϊ��\n";
    info += "E3: Please select the input mode (lex file/source file)\n";
    info += "[Run]��ͼû��ѡ������ģʽ\n";
    info += "E4: Please switch to SOURCE mode!\n";
    info += "[Run]��ͼѡ���ִ�г���ʱû���л���SOURCEģʽ\n";
    info += "E5: Please select the path for the EXE\n";
    info += "[Run]��ͼ�ı��������ΪԴ����ʱ, û����ѡ���ִ�г����·��\n";
    info += "E6: Please generate the SLR1 parsing table before performing syntax analysis.\n";
    info += "[Run]��ͼ, �ķ�����ǰû������SLR1���������﷨����\n";
    info += "E7: The statement you entered is not grammatically correct and therefore cannot be accepted.\n";
    info += "[Run]��ͼ, ���벻�����﷨, ��������\n";
    info += "E8: Please select the PROPER input mode (lex file/source file)\n";
    info += "[Run]��ͼѡ�������ģʽ���ı����ı���ƥ��\n";

    QDialog dialog;
    dialog.setWindowTitle("ʹ��˵��");
    dialog.setFixedSize(800, 600); // ����QDialog�Ĵ�СΪ400x300����

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    QTextEdit* textEdit = new QTextEdit(&dialog);
    textEdit->setPlainText(info);
    textEdit->setReadOnly(true);
    layout->addWidget(textEdit);

    QPushButton* okButton = new QPushButton("ȷ��", &dialog);
    layout->addWidget(okButton);

    QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}


