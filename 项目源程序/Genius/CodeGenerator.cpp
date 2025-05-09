/**
*****************************************************************************
*  Copyright (c) 2024 Liang Haifeng. All rights reserved
*
*  @file    CodeGenerator.cpp
*  @brief   ���ļ��Ǵʷ��������Ĵ����ļ�, �����ʷ�����������ģ��ľ���ʵ��
*           1. ������ʽתNFA
*           2. NFAתDFA
*           3. DFA��С��
*           4. Դ�������������
*           5. ���ӻ������߼�����
*           
*
*  @author  ������
*  @date    2024-07-29
*  @version V3.0 20240729
*

*
*----------------------------------------------------------------------------
*  @note ��ʷ�汾  �޸���Ա    �޸�����    �޸�����
*  @note v1.0      ������      2024-5-5    ��ɱ�д
*  @note v2.0      ������      2024-5-28   ���ƴ���, ʹ������minic
*  @note v3.0      ������      2024-7-29   �޸�����minic�ʷ�����Դ��������, ʹ����ʶ��ע��;ͬʱ֧�־�Ĭ����cpp�ļ��ɴʷ�����Դ����
*****************************************************************************
*/




#include "CodeGenerator.h"
#include"util.h"
#include<regex>
#include<cmath>
#define maxsize 200  //��ʾNFA��DFA�ľ����С
vector<edgeNode> edge;// �߼�����
vector<vertNode> vert;//�㼯����
char matrix[maxsize][maxsize];//��ȡ�߼�����Ϊ�ڽӾ��󷽱��Ʊ�
char matrix2[maxsize][maxsize];//DFA�ڽӾ���
string metachar = "";// ������ʽ��Ԫ�ַ�, �� l(l|d)* ��, l��d����Ԫ�ַ�
int BEGIN, END = 0; // BEGIN ��ʾ NFAͼ��ʼ������(һ����0); END��ʾNFAͼĩβ�Ľ��
int num3 = 0; // ��СDFA�Ľ�����
queue<int> Queue1;//�������nfa״̬ת����Ķ���,���ڹ���
set<int> Move;//����ת��dfa�Ķ��У����ڼ���move����
map<char, set<int>> Map;//����nfa״̬ת�������
map<int, int> na;//��С��dfa����,�������̬����̬
vector<map<char, int>> vecp;//������ʽ��״̬ת����
map<int, int> isTer;// ��С��DFA�ж��Ƿ�Ϊ��̬����
map<int, string> tokens;//vecpÿһ�����Ӧ��token��ӳ�� 
int vid = 2;//vecp״̬ת����ÿ�ж�Ӧ��token�±�;
map<char, string> w2l;//���ʵ�Ԫ�ַ���ӳ�亯��
map<string, string> Regex;//������������ʽ��first����������ʽ�󲿣� second��ʾ�Ҳ�
map<string, string> rangeRegex;//�����������ɷ�Χ������������ʽ��first����������ʽ�󲿣� second��ʾ�Ҳ�(�������Ҳ���'~'�ı��ʽ�뺬|�ı��ʽ)
queue<string> notSGN;//�ɷ��������+�����ַ�����Ҳ���������ʽ�����ɵ����
map<string, string> keywdReg;//��|�ķ�Χ���ʽ, ��������д�ؼ���
map<string, int> notEdge;//���з��������DFA�ı�
//map<string, int> lookup;//���ұ��������������ӳ��
map<string , string> EDGE;// ״̬ת����ĺ��ᣬ����DFA�еı�
//map<string, Token> reToken;//������ʽʶ�������num��type
//map<string, std::smatch> lmatch;//������������ʽ�󲿼����Ӧ����Ϣ����������code
queue<regexLeft> leftregexs;//������������ʽ�󲿼����Ӧ����Ϣ����������code
string exename = "";//���ɵĴʷ�����Դ�����ļ���
string newLexPath = "";//����������ʽ���ɵ�cpp�ļ���ַ
string exePath = "";//ѡ��ִ��exe�ľ��Ե�ַ
string exeRePath = "";//ѡ��ִ��exe����Ե�ַ��ַ
CodeGenerator::CodeGenerator(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
}

CodeGenerator::~CodeGenerator()
{}


// ��������: �﷨������ļ�����
// ��������: �ڿ��ӻ������д��﷨�����ļ�����ȡ���ı��������
// ��������: ��
// ��������ֵ :��
void CodeGenerator::openfile()
{
    ui.inputplainTextEdit->clear();
    QFileDialog* f = new QFileDialog(this);//�����Ի���ʵ��
    f->setWindowTitle("choose the data file*.txt");// �Ի������Ͻ���ʾ��
    f->setNameFilter("*.txt");//ɸѡ�ļ�����
    f->setViewMode(QFileDialog::Detail);

    QString filePath;
    if (f->exec() == QDialog::Accepted)// ���ȷ���˴��ļ����ɹ�, �Ͷ�ȡ�ļ���
        filePath = f->selectedFiles()[0];

    QFile file(filePath);


    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QErrorMessage* dialog = new QErrorMessage(this);
        dialog->setWindowTitle("ERROR Message");
        dialog->showMessage("Failed to open!");
        return;
    }
    QTextStream readStream(&file);

    //����Ϊ���ļ�������ʾ���ı�����
    while (!readStream.atEnd())
    {
        ui.inputplainTextEdit->appendPlainText(readStream.readLine());

    }
}


// ��������: �﷨���򱣴��ļ�����
// ��������: �����ڿ��ӻ����������ı���������﷨����
// ��������: ��
// ��������ֵ :��
void CodeGenerator::savefile()
{

    QFileDialog dlg(this);

    //��ȡ���ݵı���·��
    //QString fileName = dlg.getSaveFileName(this, tr("Save As"), "./", tr("TXT File(*.txt)"));
    QString fileName = QFileDialog::getSaveFileName(NULL, QStringLiteral("TXT SaveAs"), "newfile.txt", QString(tr("TXT File (*.txt)")),
        Q_NULLPTR, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (fileName == "")
    {
        return;
    }

    //���ݱ��浽·���ļ�
    QFile file(fileName);

    //���ı���ʽ��
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file); //IO�豸����ĵ�ַ������г�ʼ��

        out << ui.inputplainTextEdit->toPlainText() << endl; //���

        QMessageBox::warning(this, tr("Finish"), tr("Successfully save the file!"));

        file.close();
    }
    else
    {
        QMessageBox::warning(this, tr("Error"), tr("File to open file!"));
    }

}


// ��������: ��ִ���ļ�ѡ����
// ��������: �ԶԻ������ʽ����򿪱�����ɲ����Խ��дʷ������Ŀ�ִ���ļ�
// ��������: ��
// ��������ֵ :��
void CodeGenerator::choexe()
{

    QFileDialog* f = new QFileDialog(this);
    f->setWindowTitle("choose the data file *.exe");
    f->setNameFilter("*.exe");
    f->setViewMode(QFileDialog::Detail);

    QString filePath;//��ȡ���ļ��ľ���·��
    if (f->exec() == QDialog::Accepted)
        filePath = f->selectedFiles()[0];

    exePath = filePath.toStdString();// ����exe�ļ��ľ���·��
    exeRePath = exePath.substr(exePath.find_last_of('/') + 1, exePath.size());// ����ȡ�ļ���(����׺)
    QString qexepath = QString::fromStdString(exeRePath);
    ui.exeChotextEdit->setText(qexepath);

    //cout << exePath << endl;

    //����һ�ݵ���ǰ�ļ��з����������
    char cwd[256];
    _getcwd(cwd, 256);
    string dst(cwd);
    dst = " " + dst;
    string src = exePath;
    replace(src.begin(), src.end(), '/', '\\');
    string cmd = "copy " + src + dst;
    system(cmd.c_str());
}


// ��������: Դ����ѡ����
// ��������: �ԶԻ������ʽ�������Ҫ�ʷ�������Դ����
// ��������: ��
// ��������ֵ :��
void CodeGenerator::chosrccode()
{
    ui.readSrcplainTextEdit->clear();
    QFileDialog* f = new QFileDialog(this);
    f->setWindowTitle("choose your source code files");
    //f->setNameFilter("*.txt");
    f->setViewMode(QFileDialog::Detail);

    QString filePath;
    if (f->exec() == QDialog::Accepted)
        filePath = f->selectedFiles()[0];

    string srcPath = filePath.toStdString();// ����Դ�����ļ��ľ���·��
    QString qsrcpath = QString::fromStdString(srcPath.substr(srcPath.find_last_of('/') + 1, srcPath.size()));
    ui.srcChotextEdit->setText(qsrcpath);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QErrorMessage* dialog = new QErrorMessage(this);
        dialog->setWindowTitle("ERROR Message");
        dialog->showMessage("Failed to open!");
        return;
    }
    QTextStream readStream(&file);
    while (!readStream.atEnd())
    {
        ui.readSrcplainTextEdit->appendPlainText(readStream.readLine());

    }
}


// ��������: ���ʱ��뺯��
// ��������: ����ѡ�еĿ�ִ���ļ���Դ������е��ʱ���, �������������ı�����
// ��������: ��
// ��������ֵ :��
void CodeGenerator::wordsEncode()
{
    ui.wordsEcplainTextEdit->clear();
    //1. �Ƚ��ı������ļ����浽ָ���������ļ� in.txt��
    QString fileName = QString::fromStdString("in.txt");
 
    QFile file(fileName);

    //���ı���ʽ��
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file); //IO�豸����ĵ�ַ������г�ʼ��

        out << ui.readSrcplainTextEdit->toPlainText() << endl; //�����in.txt��

        //QMessageBox::warning(this, tr("Finish"), tr("Successfully save the file!"));
        file.close();
    }
    else
    {
        QMessageBox::warning(this, tr("Error"), tr("File to open file!"));
    }


    //2. ����exe�ļ�
    //cout << "exec: " << exeRePath.c_str() << endl;
    system(exeRePath.c_str());

    //3. ɾ��exe�ļ�, ����ȡnewLex.lex�ļ����ݵ��Ҳ��ı�����
    string delcmd = "del " + exeRePath;
    system(delcmd.c_str());

    string lexPath = "newLex.lex";// newLex.lex�ļ������·��
    QString qlexPath = QString::fromStdString(lexPath);

    QFile file2(qlexPath);
    if (!file2.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QErrorMessage* dialog = new QErrorMessage(this);
        dialog->setWindowTitle("ERROR Message");
        dialog->showMessage("Failed to open!");
        return;
    }
    QTextStream readStream(&file2);
    while (!readStream.atEnd())
    {
        ui.wordsEcplainTextEdit->appendPlainText(readStream.readLine());

    }
    
}

void CodeGenerator::backToMenu1()
{
    this->hide();
    emit signalBackToMenu1();
}



// ��������: ������ʽ�������
// ��������: ������DOS����������ս��д����������ʽ, ������������ʽ�����Ƿ�����
// ��������: ��
// ��������ֵ :��
void regexInfo()
{

    //cout << "������ɵ�������ʽ����:" << endl;
    //cout << "��ͼ���ʽ:  " << endl;
    for (auto p : Regex)
    {
        //cout << p.first << "   =   " << p.second << endl;

    }

    //cout <<endl<<endl <<"��Χ���ʽ:  " << endl;
    for (auto p : rangeRegex)
    {
        //cout << p.first << "   =   " << p.second << endl;

    }


    //cout << endl << endl;
}

// ��������: ������ʽ������
// ��������: ����������ʽ, �����������Ҳ�����Ϣ������в�ͬ�Ĵ���, ��������������ʽ�Ҳ�
// ��������: string ������������ʽ, ���������Ҳ�
// ��������ֵ :string ������ʽ�Ҳ�
string getReg(string s)
{
    int i = 0;

    int nbegin;//������ʽ��ͷ
    while (s[i] == ' '||s[i]=='\n'||s[i]=='\t')//�����հ�, ���з�, �Ʊ��
        i++;
    int lbegin = i;
    while (s[i] != '=')// �ҵ���=�������Ա�ָ�������ʽ�����Ҳ�
    {
        i++;
    }
    string right = s.substr(i + 1, s.size() - i - 1);//�������ص��Ҳ�


    string  left = s.substr(lbegin, i - lbegin);//������ʽ��
    left.erase(std::remove(left.begin(), left.end(), ' '), left.end());
    std::regex pattern = regex(R"((\D+)(\d+)(\D+)?)");//����ƥ���������Ҫ����NFA��������ʽ
    std::smatch match;
    string leftname = "";

    if (std::regex_search(left, match, pattern))//��������ID100name ��������NFA��
    {
        leftname = match[1].str();
        Regex[leftname] = right;
        tokens[vid] = leftname;
        vid += 1;

        string name = leftname;
        string num = match[2].str();
        string val = match[3].str();
        leftregexs.push(regexLeft(name, num, val));
    }
    else// ��������digit = [0-9]�����γɷ�Χ������
    {
        right.erase(std::remove(right.begin(), right.end(), ' '), right.end());
        int id = right.find("~");
        if (id != string::npos)//����"~"��������ķ�Χ������ʽ
        {
            notEdge[left] = 1; //Ϊ��'~'��DFA����һ����ע
            char c = right[id + 1];
            string tCode = str_format("    if((flag == 1) && (c != '%c'))\n      return \"%s\"; \n", c,left.c_str());
            notSGN.push(tCode);
        }
        else if (right.find('|') != string::npos)//����"|"��������ķ�Χ������ʽ
        {
            keywdReg[left] = right;
        }
        else 
           rangeRegex[left] = right;
        right = "";
    }


    return right;

}

// ��������: ���ӻ����������߼�����
// ��������: ���ÿ��ӻ�����ĵ�ִ���߼�
// ��������: ��
// ��������ֵ :��
void CodeGenerator::test()
{
    init();

    exename = ui.ipSrctextEdit->toPlainText().toStdString();
    //cout << "exename: " << exename << endl;
    QTextDocument* doc = ui.inputplainTextEdit->document();
    int blocks = doc->blockCount();
    QString reg = "";
    string input = "";
    //���ж�ȡ����Ķ���������ʽ��������֯��NFA����ģ������ĸ�ʽ
    for (int i = 0; i < blocks; i++)
    {
        QTextBlock textblock = doc->findBlockByNumber(i);
        string temp = textblock.text().toStdString();
        string regR = getReg(temp);//������ʽ�Ҳ�
        if (regR == "") continue;
        input +=  regR+ "@"; //����reg ͨ����@�����ָ�
        //input += textblock.text().toStdString();
        
    }


    //regexInfo();
    //cout << input << endl;



    //��������ʽת��NFAͼ
    r2Graph(input);


    //���NFAͼ
    //table1

    
    map<int, map<char, set<int>>> table1 = tableINFO1();

    ui.nfatableWidget->clear();
    int cols = table1.begin()->second.size();
    int rows = table1.size();
    ui.nfatableWidget->setColumnCount(cols + 1);
    ui.nfatableWidget->setRowCount(rows);
    QStringList title = QStringList();
    title << "";
    //cout << "NFA��ͷ����" << endl;
    for (auto c : table1.begin()->second)
    {
        //string s = "";
        //s.push_back(c.first);
        //title << s.c_str();
        //cout <<"++" << w2l[c.first] <<"++"<< endl;
        title <<w2l[c.first].c_str();
    }
    ui.nfatableWidget->setHorizontalHeaderLabels(title);
    int rownum = 0;
    for (auto it = table1.begin(); it != table1.end(); ++it)
    {
        ui.nfatableWidget->insertRow(rownum);//�ڱ��������һ��
        ui.nfatableWidget->setItem(rownum, 0, new QTableWidgetItem(QString::number(it->first)));

        int colnum = 1;
        for (auto p : it->second)
        {
            if (p.second.size() != 0)
                ui.nfatableWidget->setItem(rownum, colnum, new QTableWidgetItem(set_str(p.second).c_str()));
            colnum += 1;
        }
        rownum += 1;
    }


    //���DFAͼ
    //table2

    //��NFAͼת��DFAͼ
    map<set<int>, map<char, set<int>>> table2 = tableINFO2();

    ui.dfatableWidget->clear();
    cols = table2.begin()->second.size();
    rows = table2.size();
    ui.dfatableWidget->setColumnCount(cols + 1);
    ui.dfatableWidget->setRowCount(rows);

    //�����ͷ
    w2l['?'] = "OPT";
    //cout << "DFA��ͷ����" << endl;
    for (auto c : table2.begin()->second)
    {
        //string s = "";
        //s.push_back(c.first);
        //title << s.c_str();
        //cout << "|" << w2l[c.first] << "|" << endl;
        //string t = w2l[c.first].c_str();
    }



    QStringList title2 = QStringList();
    title2 << "";
    for (auto c : table2.begin()->second)
    {
        //string s = "";
        //s.push_back(c.first);
        //title2 << s.c_str();
        title2 << w2l[c.first].c_str();
    }
    ui.dfatableWidget->setHorizontalHeaderLabels(title2);


    rownum = 0;
    for (auto it = table2.begin(); it != table2.end(); ++it)
    {
        ui.dfatableWidget->insertRow(rownum);//�ڱ��������һ��
        ui.dfatableWidget->setItem(rownum, 0, new QTableWidgetItem(set_str(it->first).c_str()));//д���б�ͷ

        int colnum = 1;
        for (auto p : it->second)
        {
            if (p.second.size() != 0)
                ui.dfatableWidget->setItem(rownum, colnum, new QTableWidgetItem(set_str(p.second).c_str()));
            colnum += 1;
        }
        rownum += 1;
    }


    //�����С�����DFAͼ
    //table 3
    tableINFO3();
    ui.dfamtableWidget->clear();
    cols = vecp[1].size();
    rows = vecp.size();
    ui.dfamtableWidget->setColumnCount(cols + 1);
    ui.dfamtableWidget->setRowCount(rows);





    //�����ͷ
    QStringList title3 = QStringList();
    title3 << "";
    for (auto c : vecp[1])
    {
 /*       string s = "";
        s.push_back(c.first);
        title3 << s.c_str();*/
        title3 << w2l[c.first].c_str();
    }
    ui.dfamtableWidget->setHorizontalHeaderLabels(title3);


    rownum = 0;
    for (int i = 1; i < vecp.size(); i++)
    {
        ui.dfamtableWidget->insertRow(rownum);//�ڱ��������һ��
        //ui.dfamtableWidget->setItem(rownum, 0, new QTableWidgetItem(QString::number(i)));//д���б�ͷ

        if (isTer[i] == 1)
        {
            ui.dfamtableWidget->setItem(rownum, 0, new QTableWidgetItem(str_format("(%d)",i).c_str()));//д���б�ͷ
        }
        else
            ui.dfamtableWidget->setItem(rownum, 0, new QTableWidgetItem(QString::number(i)));//д���б�ͷ

        
        int colnum = 1;
        for (auto p : vecp[i])
        {
            if (p.second != -1)
                ui.dfamtableWidget->setItem(rownum, colnum, new QTableWidgetItem(QString::number(p.second)));
            colnum += 1;
        }
        rownum += 1;
    }


    //����Դ����
    geneCPP();

    //��Դ�����������
    string srcPath = newLexPath.substr(newLexPath.find_last_of('\\')+1,newLexPath.size());
    QFile file(srcPath.c_str());

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QErrorMessage* dialog = new QErrorMessage(this);
        dialog->setWindowTitle("ERROR Message");
        string message = "Failed to open!" + srcPath;
        dialog->showMessage(message.c_str());
        return;
    }

    QTextStream readStream(&file);
    while (!readStream.atEnd())
    {
        ui.codeplainTextEdit->appendPlainText(readStream.readLine());

    }


    //
    if (!geneEXE())
    {
        string s = "Can't Generate .exe from .cpp due to Lack Of Complier.\n Please try to complie it on your own" ;
        QMessageBox::warning(this, tr("Message"), tr(s.c_str()));
    }
    else
        QMessageBox::warning(this, tr("Finish"), tr("CODES ARE GENERATED SUCCESSFULLY!"));

}

// ��������: Ԫ�ַ�(�����ս������ս��)ɨ�躯��
// ��������: ɨ��������ʽ�漰������Ԫ�ַ�, ����������뵽metachar��
// ��������: string ��ʾ������ʽ���ַ���
// ��������ֵ: ��
void scanMetachar(string s)
{
    metachar = "";
    char tmp = ' ';
    for (auto c : s)
    {
        if (tmp == '@')
        {
            metachar += c;
            continue;
        }
        if (metachar.find(c) != string::npos  || c == '@'||getPriority(c)!=0)
        // �������������ʽ�е��ַ�, ����ַ������ַ����ս����������ʽ�����, ת���ַ�/������
        // ��ΪԪ�ַ���Ϊ���
        {
            tmp = c;
            continue;
        }
        tmp = c;
        metachar += c;
        
    }
    metachar += '#';//��NFAͼ��epsilonҲ��ΪԪ�ַ�
}

// ��������: ������ʽ���������˳����
// ��������: ��ȡ������ʽ�����������˳����ַ�
// ��������: char �����ַ�
// ��������ֵ: int ��������ȼ�
int getPriority(char op)
{
    if (op == '(')
        return 1;
    else if (op == '|' || op == '&')
        return 2;
    else if (op == '*' || op == '+'|| op == '?')
        return 3;
    else if (op == ')')
        return 4;
    else
        return 0;
}



// ��������: ���ͼ���ת�ַ�������
// ��������: �����ͼ���Ԫ�ض�ȡ��string
// ��������:set<int> ���ͼ���
// ��������ֵ: string  ���ͼ��ϵ�string��ʾ
string set_str(set<int> o)
{
    string result = "{";
    for (auto a : o)
    {
        result += to_string(a) + ", ";
    }
    result.erase(result.end() - 1); // ɾ��ĩβ��д���','
    result += "}";
    return result;

}

// ��������: DFA״̬���ֺ���
// ��������: ����С��DFA������, DFAԭ�Ƚ��������������(������̬����������), �˺������ڶԽ�㼯�Ͻ��л���
// ��������: set<int> DFAͼ��㹹�ɵļ���; char Ԫ�ַ�
// ��������ֵ: set<set<int>> ��㼯�ϵļ���, ���ֽ��
set<set<int>> divide(set<int> s, char c)
{
    map<int, set<int>> m;
    for (auto a : s)
    {
        int flag = 0;//�жϵ�ǰ�Ƿ�ᱻ���ֵ�0(��̬��֧)��1(����̬��֧),����3(�ޱ仯��֧)�ı�־,Ϊ0��Ữ��Ϊ3
        for (int j = 0; j < num3; j++)
        {
            //cout << "(" << a << "," << j << ")" << ":" << matrix2[a][j] << endl;
            if (matrix2[a][j] == c)
            {
                flag = 1;
                m[na[j]].insert(a);
                //cout << na[j] << "֧����" << a << endl;
            }
        }
        if (flag == 0)
        {
            m[3].insert(a);
            //cout <<"�޹�֧����" << a << endl;
        }
    }

    set<set<int>> re;
    
    //cout << "����" << set_str(s) << "����" << c << "��������:" << endl;

    for (auto p : m)
    {
        set<int> t = p.second;
        //cout << set_str(t) << endl;
        re.insert(p.second);
    }
    return re;
}

// ��������: DFA��С��������
// ��������: ��DFAͼ������С��
// ��������: queue<set<int>>& q ���Ͻ���������; set<set<int>>& result ���ջ��ֵĽ��
// ��������ֵ:��
void process(queue<set<int>>& q, set<set<int>>& result)
{

    set<int> temp = q.front();
    //cout << endl << endl << endl;
    //cout << "��������� temp:" << set_str(temp) << endl;
    q.pop();
    for (auto c : metachar)
    {
        // ���Ԫ�ַ�����, ����ǰ����Ľ�㼯���Ƿ�ɻ���
        if (temp.size() == 1)// ���Ͻ��ֻ��һ���Ĳ��軮��
            break;
        set<set<int>> out = divide(temp, c); 
        if (out.size() >1 )// ��㼯�ϵļ��ϴ���1˵���ɷ�, �»��ֵ����ɽ�㼯�Ͻ����������
        {
            //cout << "���·�֧��ӣ�" << endl;
            for (auto t : out)
            {
                //cout << "���:" << set_str(t) << endl;
                q.push(t);
            }
            return;
        }
    }

    result.insert(temp);
    //for (auto i : result)
        //cout << set_str(i) << "  ";

}

// ��������: ��ʼ������
// ��������: ��������������ĸ���������ȫ�ֱ�����ʼ��
// ��������: ��
// ��������ֵ: ��
void CodeGenerator::init()
{
    edge.clear();
    vert.clear();
    memset(matrix, 0, sizeof(matrix));
    memset(matrix2, 0, sizeof(matrix2));
    metachar = "";
    BEGIN = 0; END = 0;
    num3 = 0;
    while (!Queue1.empty())
        Queue1.pop();
    Move.clear();
    Map.clear();
    na.clear();
    vecp.clear();
    isTer.clear();
    tokens.clear();
    vid = 2;
    w2l.clear();
    Regex.clear();
    rangeRegex.clear();
    while (!notSGN.empty())
        notSGN.pop();
    EDGE.clear();
    while (!leftregexs.empty())
        leftregexs.pop();
    exename = "";
    newLexPath = "";
    exePath = "";
    exeRePath = "";
    ui.nfatableWidget->clear();
    ui.dfatableWidget->clear();
    ui.dfamtableWidget->clear();
    ui.codeplainTextEdit->clear();

}

// ��������: NFA�����㺯��
// ��������: ���ݴ���������ʽ�������NFA������ջ�������㴦��, �˴������������˵Ϊ����Ӧ������ʽ��NFAͼ
// ��������: stack<edgeNode> NFA������ջ; char ������ʽ�����
// ��������ֵ: ��
void calculate(stack<edgeNode>& mystack, char operation)
{
    edgeNode left, right, result;
    right = mystack.top(); // ��������ջ����Ϊ�Ҳ�����
    mystack.pop();
    switch (operation)
    {
        case '*':// ���бհ�����, �����Ҳ���������
        {
            vertNode start = vertNode(vert.size(), 0, 2); vert.push_back(start);
            vertNode end = vertNode(vert.size(), 2, 0); vert.push_back(end);//���Ԫ�ַ����ɶ�Ӧ����
            edgeNode e1 = edgeNode(start.nodenum, right.start); edge.push_back(e1);
            edgeNode e2 = edgeNode(right.end, right.start); edge.push_back(e2);
            edgeNode e3 = edgeNode(right.end, end.nodenum); edge.push_back(e3);
            edgeNode e4 = edgeNode(start.nodenum, end.nodenum); edge.push_back(e4);//���Ԫ�ַ����ɶ�Ӧ�߽��

            result = edgeNode(start.nodenum, end.nodenum);
        }break;

        case '+':// �������հ�����, �����Ҳ���������
        {
            vertNode start = vertNode(vert.size(), 0, 2); vert.push_back(start);
            vertNode end = vertNode(vert.size(), 2, 0); vert.push_back(end);//���Ԫ�ַ����ɶ�Ӧ����
            edgeNode e1 = edgeNode(start.nodenum, right.start); edge.push_back(e1);
            edgeNode e2 = edgeNode(right.end, right.start); edge.push_back(e2);
            edgeNode e3 = edgeNode(right.end, end.nodenum); edge.push_back(e3);//���Ԫ�ַ����ɶ�Ӧ�߽��
            result = edgeNode(start.nodenum, end.nodenum);
        }break;

        case '?':// ���п�ѡ����, �����Ҳ�����
        {
            vertNode start = vertNode(vert.size(), 0, 2); vert.push_back(start);
            vertNode end = vertNode(vert.size(), 2, 0); vert.push_back(end);//���Ԫ�ַ����ɶ�Ӧ����
            edgeNode e1 = edgeNode(start.nodenum, right.start); edge.push_back(e1);
            edgeNode e2 = edgeNode(start.nodenum, end.nodenum, '?'); edge.push_back(e2);
            edgeNode e3 = edgeNode(right.end, end.nodenum); edge.push_back(e3);
            metachar += "?";
            result = edgeNode(start.nodenum, end.nodenum);
        }break;

        case '&':// ����������, ��Ҫ��һ������NFA����ջջ��Ԫ����Ϊ���������������
        {
            left = mystack.top();
            mystack.pop();

            edgeNode e = edgeNode(left.end, right.start);
            edge.push_back(e);
            result = edgeNode(left.start, right.end);
        }break;

        case '|':// ���л�����, ��Ҫ��һ������NFA����ջջ��Ԫ����Ϊ���������������
        {
            left = mystack.top();
            mystack.pop();
            vertNode start = vertNode(vert.size(), 0, 2); vert.push_back(start);
            vertNode end = vertNode(vert.size(), 2, 0); vert.push_back(end);//���Ԫ�ַ����ɶ�Ӧ����
            edgeNode e1 = edgeNode(start.nodenum, left.start); edge.push_back(e1);
            edgeNode e2 = edgeNode(start.nodenum, right.start); edge.push_back(e2);
            ////cout << left.start << "====" << left.end<< endl;
            edgeNode e3 = edgeNode(left.end, end.nodenum); edge.push_back(e3);
            edgeNode e4 = edgeNode(right.end, end.nodenum); edge.push_back(e4);

            result = edgeNode(start.nodenum, end.nodenum);
        }break;
    }

    mystack.push(result);// �����ս���Ż�NFA����ջ
}

//�������ƣ� ����תԪ�ַ�����
//�������ܣ� ���������ȡ���ʲ�������ӳ�䵽Ԫ�ַ����Է�����к�����������
//���������� ������������ʽ��ɵ�һ���ַ���
//��������ֵ�� ���е���-�ַ�ӳ����һ���ַ���, ������'@'�ָ�, ���ǻ����������
string word2letter(string INPUT)
{
    map<string, int> exist;//�жϵ����Ƿ��Ѿ����ֹ��ĺ���
   
    char key = ' ';//ÿ��Ӣ�ĵ�����w2lӳ���еĹؼ�ֵ
    
    queue<char> alpha;// ����26��Ӣ����ĸ��Сд�Ķ���
    for (int i = 97; i <= 122; i++)
        alpha.push(char(i));
    for (int i = 65; i <= 90; i++)
        alpha.push(char(i));
    string reg = "";
    int rpos = 0;
    int begin = 0;
    while ((rpos = INPUT.find('@', begin)) != string::npos && begin < INPUT.size())
    {
       // �������������������ʽ(��Щ���ʽ����'@'�������������˳���INPUT��)
        string input = INPUT.substr(begin, rpos - begin);// ��Сɨ�跶Χ
        string newreg = "";//����ӳ��֮���������ʽ
        //cout << "������������䣺" << input << endl;
        begin = rpos + 1;
        int pos = 0;
        while (pos < input.size())
        // ������ʽinput�е��ַ�
        {

            while (pos < input.size() && (input[pos] == 32 || input[pos] == 9))//�����ո��tab
                pos += 1;

            if (isalpha(input[pos]))// �����ǰ���������ַ���Ӣ����ĸ����뵥��ƴ�ӹ���
            {
                int beginpos = pos;
                key = ' ';
                while (isalnum(input[pos]) || input[pos] == '_')// ��ĸ, ����, �»�����Ϊ�Ϸ�������ɲ��ֲ�������
                {
                    pos++;
                }

                string word = input.substr(beginpos, pos - beginpos);// ����pos����������ʽ���и������

                if ((!alpha.empty()) && exist[word] == 0)// �����СдӢ����ĸ��û�б�ӳ�����ҵ�ǰ����û�г��ֹ��ͽ���ӳ��
                {
                    key = alpha.front();
                    alpha.pop();
                    w2l[key] = word;// ��һ������ӳ��Ϊһ����ĸ
                    exist[word] = key;
                }
                newreg += char(exist[word]);
            }
            else if (input[pos] != '\0')// ��������Ӣ����ĸ�ҷ��ַ����ս�����������ַ�������
            {
                char t = input[pos];
                if (getPriority(t) == 0)// �����������ʽ�����
                {
                    string tt = "";
                    tt += t;
                    w2l[t] = tt;// �����ı�, �Լ�ӳ�䵽�Լ�
                }
                    
                newreg += t;
                pos++;
            }
        }
        reg += "(" + newreg + ")|";//��ǰ�ַ���������Ϻ�, ��Ҫ�ͺ����ӳ���INPUT����������������ʽ������
    }
    //cout << "����Ϊ��ĸ����ӳ���" << endl;
    for (auto p : w2l)
    {
        //cout << "(����)" << p.first << "------>" << "(��ĸ)" << p.second << endl;
    }
    //cout << endl;
    reg.pop_back();//ɾ��ĩβ����Ĳ��������
    //cout <<"���֮���������ʽ" << reg << endl;
    return reg;

}


// ��������: ������ʽתNFAͼ����
// ��������: ��������ʽתΪNFAͼ
// ��������: string ����������ʽͨ��'@'�νӶ��ɵĳ���
// ��������ֵ: NFAͼ��β������Ӷ��ɵı߽��
edgeNode r2Graph(string str)
{
    str = word2letter(str);// ��ԭ���ɵ�����ɵ�������ʽӳ�����Ԫ�ַ���ɵ�������ʽ

    scanMetachar(str);// ɨ�������Ԫ�ַ�
    //cout << "����Ԫ�ַ�:" << metachar << endl;
    //������׺���ʽ,Ĭ�������ǺϷ���  
    stack<edgeNode> mystack_metachar;//������ջ,�ݴ�Ԫ�ַ�����������֮����м���,�Ա߽�����ʽ
    stack<char> mystack_operation;//����ջ, ����Ԫ��һ���ȵײ�Ԫ�����ȼ���, ���������ջԪ�����ȼ�����ջ��Ԫ��, ִ��ѭ����������, ֱ����ջԪ��Ϊ��ǰ���ȼ����
    int i = 0;//��������������ʽÿһ���ַ���ָ��
    int size = str.size();//��ȡ������ʽ����
    char tmp_operation;// �������ջջ����ǰջ�����������ʱ����
    string tmp_num;
    //vert.push_back(vertNode(vert.size(), 0, 0));
    while (i < size)
    {
        if (getPriority(str[i]) == 0)//����ָ��ָ��Ԫ�ַ��Ĵ���: ֱ������NFA�߽��(��Ϊ��ǰԪ�ַ�,��Ϊ�������)
        {
            if (str[i] == '@')//  '@' Ϊת�������
            {
                i += 1;
            }//���������
            vertNode start = vertNode(vert.size(), 0, 1); vert.push_back(start);
            vertNode end = vertNode(vert.size(), 1, 0); vert.push_back(end);//���Ԫ�ַ����ɶ�Ӧ����
            edgeNode temp = edgeNode(start.nodenum, end.nodenum, str[i]);//���Ԫ�ַ����ɶ�Ӧ�߽��
            edge.push_back(temp);

            //����Ϣ�Ǽ��ڵ㼯��߼�������
            mystack_metachar.push(temp);//��������ջ
            if ((i + 1 < size) && ((getPriority(str[i + 1]) == 0) || str[i + 1] == '('))
            //if ((i + 1 < size) && ((metachar.find(str[i+1]!=string::npos)) || str[i + 1] == '('))
            //if ((i + 1 < size) && ((isalpha(str[i + 1]) != 0) || str[i + 1] == '('))
            //��ǰɨ���ַ�������һ���ַ���ΪԪ�ַ�, ˵������������ ������һ���ַ�Ϊ(, ˵����ǰ��������Ҫ��()������������������
            {
                str[i] = '&';// ����������ʽ��������û�о����ַ���ʾ, ������Ϊ�˷����������в���һ��'&'������������
                i--;

            }
            i++;

        }
        else if (str[i] == '|' || str[i] == '&' || str[i] == '*' || str[i] == '+' || str[i] == '?')// ɨ�赽������������ʽ������Ĵ���
        {
            if (str[i] == '*' || str[i] == '+' || str[i] == '?')// ����հ����ѡ��Щ��Ŀ�����
            {
                calculate(mystack_metachar, str[i]);// ������ӦNFAͼ
                //if ((i + 1 < size) && ((isalpha(str[i + 1]) != 0) || str[i + 1] == '('))
                if ((i + 1 < size) && ((getPriority(str[i + 1]) == 0) || str[i + 1] == '('))
                // ���������ȡ���ַ�ΪԪ�ַ�����������(������������, ���������ΪԪ�ַ�), ��ǰ��Ŀ����������Ҫ�������������
                {
                    str[i] = '&';
                    i--;
                }
            }
            else// �����������������ĵ�˫Ŀ�����
            {
                if (mystack_operation.empty())// �������ջ�ǿյľͲ���Ҫ�Ƚ����ȼ�, ��ǰ�ַ�ֱ�ӽ�ջ
                {
                    mystack_operation.push(str[i]);
                }
                else// �������ջΪ����Ҫ��һ������
                {
                    while (!mystack_operation.empty())// ���ϵ�������ջ�������
                    {
                        tmp_operation = mystack_operation.top();// ����ջ�������
                        if (getPriority(tmp_operation) >= getPriority(str[i]))
                            // �Ƚϵ�ǰ�ַ���ջ������������ȼ�, ���ջ�������������������������NFA������ջ
                        {
                            //����  
                            calculate(mystack_metachar, tmp_operation);
                            mystack_operation.pop();
                        }
                        else 
                            break; // �����ǰ�ַ����ȼ�����, ����ջ, ��Ϊջ��Ԫ��, �ս᱾�ε�������
                    }
                    mystack_operation.push(str[i]);
                }

            }

            i++;
        }
        else // �������������
        {
            if (str[i] == '(')// ������ֱ�������ջ
            {
                mystack_operation.push(str[i]);
            }
            else if(str[i]==')')
            {
                while (mystack_operation.top() != '(') //�����������ջ��Ԫ�ز�������ջ��Ԫ��Ϊ������
                {
                    tmp_operation = mystack_operation.top();
                    //����  
                    calculate(mystack_metachar, tmp_operation);
                    mystack_operation.pop();
                }
                mystack_operation.pop();
                if ((i + 1 < size) && ((isalpha(str[i + 1]) != 0) || str[i + 1] == '('))
                //������������Ҫ������Ԫ�ַ�����������
                {
                    str[i] = '&';
                    i--;
                }
                ////cout << "$%%$%$%$   " << mystack_metachar.top().start << "------->" << mystack_metachar.top().end << endl;
            }
            i++;
        }

    }
    //���������ջ�ǿգ���������Ԫ��  
    while (!mystack_operation.empty()) {
        tmp_operation = mystack_operation.top();
        //����  
        calculate(mystack_metachar, tmp_operation);
        mystack_operation.pop();
    }
    //cout << "topԪ��:  " << mystack_metachar.top().start << "\nendԪ�أ� " << mystack_metachar.top().end << endl;
    BEGIN = mystack_metachar.top().start;
    END = mystack_metachar.top().end;
    memset(matrix, 1, sizeof(matrix));

    // ��״̬ת��������ڽӾ���(NFA���ΪΪ��, Ԫ�ַ�Ϊ��)
    for (auto a : edge)
    {
        matrix[a.start][a.end] = a.weight;
        ////cout << "**" << matrix[a.start][a.end] << endl;
    }
    //cout << "�߼���������:" << endl;
    graphINFO();
    return mystack_metachar.top();
}

// ��������: NFAͼDOS�������뺯��
// ��������: �Ա߼��������ʽ��DOS�������NFAͼ
// ��������: ��
// ��������ֵ: ��
void graphINFO()
{
    for (auto a : edge)
    {
        //cout << a.start << "-----" << a.weight << "----->" << a.end << endl;
    }


}



// ��������: NFA״̬ת�������ɺ���
// ��������: ��r2Graph�����������NFA�߼����鴦���״̬ת����
// ��������: ��
// ��������ֵ: map<int, map<char, set<int>>> NFA״̬װ����
map<int, map<char, set<int>>> tableINFO1()
{
    map<int, map<char, set<int>>> final;// ���溯������ֵ
    map<char, set<int>> items; // ���� Ԫ�ַ� - ��㼯�� һ�Զ�ӳ�����ʱ����

    int visit[maxsize];
    memset(visit, 0, sizeof(visit));

    // DOS�������ڽӾ����ֵ�Ƿ���ȷ
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {

            //cout << matrix[i][j] << " ";
        }
        //cout << endl;
    }

    //cout << endl << endl;

    // ���NFA״̬ת�����ͷ
    //cout << "table1:" << endl;
    //cout << "        ";
    for (auto c : metachar)
    {
        //cout << c << "       ";
    }
    //cout << endl;
    Queue1.push(BEGIN);

    //��ȱ����ڽӾ���������״̬ת����, ÿһ��NFA��Ž�����Ϊ����������һ��, ÿһ�α����ͻ����Ŷ�Ӧ��һ��״̬ת����������
    while (!Queue1.empty())
    {
        int i = Queue1.front();
        //cout << endl << endl;
        //cout << "NFAд��" << i << "��" << endl;
        Queue1.pop();
        for (int j = 0; j <= END; j++)
        {
            if (metachar.find(matrix[i][j]) != string::npos)// ���ڽӾ���ֵ��Ԫ�ַ�, ����memset���õĿ�ֵ
            {
                Map[matrix[i][j]].insert(j);// Map ��Ԫ�ַ� - ��㼯�� ��һ�Զ�ӳ��, ��Ӧ(���Ƶ��ǲ�����)״̬ת�����һ��
                //cout << "��" << matrix[i][j] << "������" << j << endl;
                ////cout << "zifuchuan: " << matrix[i][j] << "------" << Map[matrix[i][j]] << endl;
                ////cout << "��ǰ��" << i << ",  ��" << j << "����" << endl;              
                if (visit[j] == 0)
                    Queue1.push(j);
                visit[j] = 1;
            }

        }

        //cout << i << ":       ";
       
        for (auto a : metachar) //��Map�����ݷŵ���ʱ����items��, ��Ϊÿ��дת����һ�о�Ҫ����һ��Map
        {
            //cout << set_str(Map[a]) << "(" << a << ")" << "    ";
            items[a] = Map[a];
            Map[a].clear();
        }
        final[i] = items; // ���շ��ʵ���NFA�����д״̬ת�����Ӧ�е���Ϣ
        items.clear();
        //cout << endl;

    }
    return final;
}


// ��������: move���㺯��
// ��������: �Ը�����㼯�ϰ��ո����ĵ�һԪ�ַ�����move����(������㼯����Ϊһ������, Move���㼴���������Ԫ�ַ���Ȩֵ�ı��ڽӵĽ����ѡ����)
// ��������: set<int> ��㼯��; char ��һԪ�ַ�
// ��������ֵ: ��
void move(set<int> begin, char metachar)
{
    Move.clear();// Move�������浱ǰMove������
    for (auto a : begin)// ���������㼯���еĽ��
    {
        int id = a;
        for (int j = 0; j <= END; j++)// 
        {
            if (matrix[id][j] == metachar) // �����ǰ���������j���ͨ��metachar�ڽ�
            {
                Move.insert(j); // ��j����������Move
            }
        }
    }
}


// ��������: �հ����㺯��
// ��������: �Դ���Ľ�㼯�Ͻ��бհ�����, ������㼯����Ϊһ������, ��հ�����ʵ���ǽ��������epsilon���ڽӵĽ����������
// ��������: set<int> ��㼯��
// ��������ֵ: set<int> �հ�������
set<int> closure(set<int> move)
{
    set<int> temp; // ������Ŵ����㼯�ϳ�ֵ����ʱ����, ��Ϊ�����move�����޸�, �����������ʱ����
    queue<int> queue;// �������
    int visited[maxsize];// ������ǽ���Ƿ񱻷��ʹ��ĸ�������, �±��ʾ���, �±��Ӧ����ֵΪ���ʱ�־
    for (auto a : move)// ���ݽ�㼯��
    {
        temp.insert(a);
    }
    for (auto node : move)// ������㼯���е����н��
    {
        queue.push(node);
        memset(visited, 0, sizeof(visited));
        while (!queue.empty())// ѭ�����������Ϊ��
        {
            int i = queue.front(); queue.pop();
            visited[i] = 1;
            for (int j = 0; j <= END; j++)
            {
                if (matrix[i][j] == '#' && visited[j] == 0)// �����ǰ���i����jͨ��epsilon�ڽ�, �����temp����, �������������
                {
                    temp.insert(j);
                    visited[j] = 1;
                    queue.push(j);
                }
            }

        }

    }
    return  temp;

}


// ��������: NFAͼתDFAͼ����
// ��������: ��NFAͼתΪDFAͼ
// ��������: ��
// ��������ֵ: ��ʾ״̬װ����ĸ�������
map<set<int>, map<char, set<int>>> tableINFO2()
{
    map<set<int>, map<char, set<int>>> final; //������������DFAͼ״̬ת���������
    map<char, set<int>> items; // Ԫ�ַ� - ״̬ ��һ�Զ�ӳ��, ��DFA״̬ת�����б�ʾĳ�е�ֵ(�������ݱ�ͷ)
    string t = metachar;
    metachar = "";
    for (auto c : t) // ���ѭ��ʵ����ʵ���ǽ�epsilon�ų���Ԫ�ַ�
    {
        if (c == '#') continue;
        metachar += c;

    }
    memset(matrix2, 1, sizeof(matrix2));
    queue<set<int>> status;//����״̬ת�����״̬��������ж���
    map<set<int>, int> history; // DFAͼһ�ж�ӦΨһ��һ����㼯��, Ҳ��״̬, ����һ���ж�״̬�Ƿ���ֹ�������
                               // ���ֹ���״̬���������ֵ�˳��, ��1��ʼ, û���ֵ�״̬Ϊ0, ������㼯��Ҳ��ӳ��Ϊһ�����
    set<int> begin; // ����DFAͼ��Ҫ��㼯�ϲ��ϵ���, �����������Ľ�㼯��, ��Ϊ��ʼ����
    int flag = 0; // �ж�NFAͼ�������Ƿ����ڽӱ�Ϊepsilon�ı�־

    for (auto c : edge)// ���ѭ����ͨ�������߼������ʾ��NFAͼ��ȷ���������Ƿ����ڽӱ�Ϊepsilon
    {
        if (c.start == BEGIN && c.weight == '#')
        {
            flag = 1;

        }
    }
    if (flag == 0)
        begin.insert(BEGIN); // ���������û��ͨ��epsilon���ӵ��������, ����ʼ���Ͻ�����������
    else
    {
        set<int> t; t.insert(BEGIN);
        begin = closure(t); // �����������ͨ��epsilon���ӵ��������, ����ʼ����Ϊ�����㾭���հ������Ľ�㼯��
    }

    num3 = 0; //DFA��Ч״̬������
    history[begin] = num3++;
    status.push(begin); 

    //cout << endl << endl;
    //cout << "table2:" << endl;
    //cout << "                 ";
    for (auto c : metachar) 
    {
        if (c == '#') break;
        //cout << c << "                                   ";
    }//�����ͷ
    //cout << endl;

    while (!status.empty()) // ѭ����״̬�������Ϊ��
    {

        set<int> temp = status.front(); // ���ﵯ����temp״̬����DFAͼȷ�����ڵ�״̬, ��Ϊȷ�����ڵĲŻ�������
        status.pop();
        //cout << set_str(temp) << "     ";
        for (auto c : metachar) // ��������Ԫ�ַ�, ��ǰ������е�����״̬���������ÿһ��Ԫ�ַ�����move����, ���űհ�����
                                // ������̾Ϳ��ܲ����µ�״̬, �µ�״̬�ͻ����״̬������еȴ�����
        {

            if (c == '#') break; 
            move(temp, c); //����move����
            set<int> clo = closure(Move); // ���бհ�����
            //cout << set_str(clo) << "(" << c << ")" << "       ";
            items[c] = clo; // ������temp��Ϊ����״̬, ����Ԫ�ַ�c�����״̬, ���ֵ�״̬ת�����Ͼ�����дtemp״̬Ϊ�кŵ�
                            // cΪ�б�ͷ��״̬ת����ֵ
            if (clo.size() == 0) continue;
            if (history[clo] == 0) // ���cloΪ��״̬, ��������ֵ���ű���ѳ���, �������������
            {
                status.push(clo);
                history[clo] = num3++;
            }
            matrix2[history[temp]][history[clo]] = c; // ����DFA�ڽӾ���, ����״̬(��㼯��)ӳ��Ϊһ�����, ����DFAͼ������
                                                      //��һ����DFA�ĵ�һ����ʾ
        }

        if (temp.find(END) != temp.end()) // ����ж����˳���DFA��С���漰����̬���������̬������������
            na[history[temp]] = 2; // �����ǰDFA��㺬NFA���ս��, ���仮����̬����
        else
            na[history[temp]] = 1; // ���������̬���
        //cout << endl;
        final[temp] = items; // ��д״̬ת������temp�е�ֵ
        items.clear();
    }
    return final;

}


// ��������: DFA��С������
// ��������: ��DFA��С��
// ��������: ��
// ��������ֵ: ��
void tableINFO3()
{
    //cout << endl << endl << endl;
    for (int i = 0; i < num3; i++) //����DFAͼ���ڽӾ�����һ��
    {
        for (int j = 0; j < num3; j++)
        {
            //cout << matrix2[i][j] << " ";
        }
        //cout << endl;
    }

    for (auto it = na.rbegin(); it != na.rend(); ++it)  // �������һ����̬���������̬����
    {
        //cout << it->first << "------" << it->second << endl;
    }

    set<int> N, A; // N��ʾ����̬����, A��ʾ��̬����
    for (auto p : na) // ��ʼ����̬���������̬����
    {
        if (p.second == 1)
            N.insert(p.first);
        else
            A.insert(p.first);
    }

    queue<set<int>> q; // ��㼯���������, 
    set<set<int>> result; // ���汾�ν�㼯�ϻ���������������
    q.push(N); q.push(A); // ��̬���������̬������Ϊ������г�ֵ, �����ĵ����л�ֳ����ɸ���С��DFAͼ��״̬
    while (!q.empty()) // ����������С��DFA��״̬
    {
        process(q, result);
    }
    //cout << endl;
    //cout << "���ֲ�Ϊ��";

    int num = 1; 
    map<int, int> m2; // ԭDFAͼ��� - ��С��DFA ���һӳ�� 

    for (auto i : result) // ���ѭ���ǽ�ԭDFAͼ�����н��ӳ�䵽��С��DFAͼ�еĽ��, �൱�ڽ��ϲ�
    {
        if (i.size() == 0) continue;
        //cout << set_str(i) << "#  ";
        for (auto n : i)
        {
            if (A.count(n))
            {
                isTer[num] = 1;
                //cout << endl<<n << "����" << num << "Ϊ��ֹ���" << endl;
            }
            m2[n] = num;
        }

        num++;
    }
    //cout << endl;

    ////cout << matrix[0][0] << "===="<<int(matrix[0][0]) << endl;

    vector<map<char, int>> vec(num);//������ʽ��״̬ת����, �±��ʾ����״̬, ֵ����״̬ת����һ�е�����
    /*for (int i = 0; i < num; i++)
    {
        vec[0]=
    }*/
    for (int i = 0; i < num3; i++) // ͨ������ �ڽӾ�����ʽ��ԭDFA ���� ��С��DFA״̬ת����
    {
        for (int j = 0; j < num3; j++)
        {
            if (int(matrix2[i][j]) != 1) // memset����1����ʼ��, ��1˵���Ǵ��ڵĽ��
            {
                vec[m2[i]][matrix2[i][j]] = m2[j]; // m2[i]��m2[j]����ԭDFA���ӳ�䵽��С��DFA�ϵĽ��, 
                                                   // m2[i]ȷ����С��DFA״̬ת������к�, matrix2[i][j]ȷ���к�, ֵ��m2[j]
                ////cout << m2[i] << "---" << matrix2[i][j] << "--->" << m2[j] << endl;

            }
        }
    }

    for (char c : metachar) // ���ѭ��ֻ����ı�һ��˳���, �����ƺ�û����, �����
    {
        for (int i = 1; i < num; i++)
        {
            if (vec[i][c] == 0)
                vec[i][c] = -1;
        }
    }


    // �����С��DFA���һ��
    //cout << endl << endl;
    //cout << "table3:" << endl;
    //cout << "                 ";
    for (auto c : metachar)
    {
        //cout << c << "                ";
    }//�����ͷ
    //cout << endl;

    for (int i = 1; i < num; i++)
    {
        //cout << i << "              ";


        //for (char c : metachar)
        //{
        //    if (vec[i][c] == -1)
        //        //cout << "other" << "             ";
        //    else
        //        //cout << vec[i][c] << "(" << c << ")" << "              ";

        //}
        ////cout << "size:" << vec[i].size() << endl;
        //cout << endl;
    }

    vecp.swap(vec);


}



// ��������: ��Χ���ʽ������
// ��������: ����[***]��ʽ��״̬���ʽ
// ��������: ofstream �ļ������; string ���ʽ��; string ���ʽ�Ҳ�
// ��������ֵ: ��

void writeRanBra(ofstream &of, string left, string right)
{
  
    //if (right.find('-') != string::npos) //��������[0-9] [a-z]�������ַ���ʾ��Χ�ı��ʽ
    if( 3*count(right.begin(),right.end(),'-')+2 == right.size()) // һ�� '-' ��Ӧ�����ս��ַ�, ������������жϵ�ǰ���ʽ�Ƿ�Ϸ�
    {
        //cout << "���ַ� reg" << right << endl;
        std::regex pattern = regex(R"((\S)(-)(\S))");
        std::smatch match;

        while (regex_search(right, match, pattern))
        {
            int beg = match[1].str()[0];
            int end = match[3].str()[0];
            if (end < beg) 
            {

                cerr << "WRONG REGEX!"<< str_format("  %c can't reach %c", match[1].str()[0], match[3].str()[0]) << endl;
                abort();
                
            }
            else
            {
                string tCode = str_format("    if((int(c) >= %d) && (int(c) <= %d))\n      return \"%s\";\n", beg,end, left.c_str());
                of << tCode << endl;
            }

            right = right.substr(right.find('-') + 2, right.size());
        }
    }
    else //�������� [abc]�Ȳ������ַ��ı��ʽ
    {
        string tCode = "    if(";     
        for (auto c : right.substr(1, right.size() - 3))
        {
            string t = str_format("c == '%c' ||", c);
            if(left=="DOUBLE")
                 notEdge[std::string{c}] = 2;
            tCode +=t;
        }
        tCode += str_format("c == '%c')", right[right.size() - 2]);
        if (left == "DOUBLE")
            notEdge[std::string{right[right.size() - 2]}] = 2;
        //tCode[tCode.size() - 1] = ')';
        if (left == "CLOSURE")
        {
            of << tCode << endl << "      return std::string{c};" << endl << endl;
        }
        else  if (left == "DOUBLE")
        {
            of << tCode << endl;
            of << "    {" << endl;
            of << "      if(flag==2)" << endl;
            of << "      return std::string{c};" << endl << endl;
            of << "    }" << endl;
        }
        else
        {
            of << tCode << endl << "      return \"" << left << "\";" << endl << endl;
        }
        

    }

        
}

// ��������: ����Χ���ʽ������
// ��������: �������� a|b|c �����ķ�Χ���ʽ
// ��������: ofstream �ļ������; string ���ʽ��; string ���ʽ�Ҳ�
// ��������ֵ: ��
void writeRanOr(ofstream& of, string left, string right)
{
    right = "|" + right;
    std::regex pattern = regex(R"((\|)([^|]+))");
    std::smatch match;
    string::const_iterator citer = right.cbegin();
    string code = "    if(";
    while (regex_search(citer, right.cend(), match, pattern))
    {
        citer = match[0].second;
        string t = str_format("s == \"%s\"||", match[2].str().c_str());
        code += t;
    }
    //code[code.size() - 1] = ')';
    code[code.size() - 2] = ')';
    code = code.substr(0, code.size()-1);
    of << code << endl << "      return \"" << left << "\";" << endl << endl;
}

// ��������: range()�������ɺ���
// ��������: ���ɱ�������е�range��������ĺ���
// ��������: ofstream ����ļ���
// ��������ֵ: ��
//
void prodRan(ofstream& of)
{
    queue<string> notReg;//����������ķ�Χ���ʽ,������if���
    queue<string> norReg;//������������ķ�Χ���ʽ

    string tCode = "";
    string left = "";
    string right = "";
    of << "string range(char c, int flag)" << endl << "{" << endl;;

    //�������ɴ���'~'��Χ������ʽ�Ĵ���
    while (!notSGN.empty())
    {
        string temp = notSGN.front(); notSGN.pop();
        of << temp<<endl;
    }


    //���� [*-*]��[***] ���͵ķ�Χ������ʽ
    for (auto ranReg : rangeRegex)
    {
        left = ranReg.first;
        right = ranReg.second;
        writeRanBra(of, left, right);

    }   


    of << "    return \"null\";"<<endl;
    of << "}" << endl<<endl<<endl;
}


// ��������: lookup()�������ɺ���
// ��������: ���ɱ�������е�lookup��������ĺ���
// ��������: ofstream ����ļ���
// ��������ֵ: ��
void proLookup(ofstream &of)
{
    of << "string lookup(string s)" << endl << "{" << endl;

    //�������� *|**|**
    for (auto regex : keywdReg)
    {

        writeRanOr(of, regex.first, regex.second);
    }
    of << "    return \"\";" << endl;
    of << "}" << endl<<endl<<endl;
}

// ��������: readSource()�������ɺ���
// ��������: ���ɱ�������е�readSource��������ĺ���
// ��������: ofstream ����ļ���
// ��������ֵ: ��
void  proRead(ofstream& of)
{
    of << "void readSource()" << endl;
    of << "{" << endl;
    of << "    char cwd[256];" << endl;
    of << "    _getcwd(cwd, 256);" << endl;
    of << "    string in = \"\\\\in.txt\";" << endl;
    of << "    string ipath = cwd + in;" << endl;

    of << "    ifstream tf(ipath);" << endl;
    of << "    if (!tf)" << endl;
    of << "    {" << endl;
    of << "    std::ofstream of(\"in.txt\", std::ios::out);" << endl;
    of << "    of << \"YOU NEED TO PREPARE THE INPUT\" << endl;" << endl;
    of << "    of.close();" << endl;
    of << "    }" << endl;


    of << "    ifstream f(ipath);" << endl;
    of << "    if (!f) {" << endl;
    of << "        cerr << \"can't read file\" << in << endl;" << endl;
    of << "        abort();" << endl;
    of << "    }" << endl;
    of << "    string line = \"\";"<<endl;
    of << "    while (std::getline(f, line))" << endl;
    of << "    {" << endl;
    of << "        content.push(line);" << endl;
    of << "    }" << endl;
    of << "    while (std::getline(f, line))" << endl;
    of << "    {" << endl;
    of << "        content.push(line);" << endl;
    of << "    }" << endl;
    of << "    f.close();" << endl;
    of << "    buff = content.front();" << endl;
    of << "    buffsize = buff.size();" << endl;
    of << "}" << endl;


}


// ��������: getNextChar()�������ɺ���
// ��������: ���ɱ�������е�getNextChar��������ĺ���
// ��������: ofstream ����ļ���
// ��������ֵ: ��
void proScan(ofstream& of)
{
    of << "char getNextChar()" << endl;
    of << "{" << endl;
    of << "    if (pos > buffsize)" << endl;
    of << "    {" << endl;
    of << "        pos = 0;" << endl;
    of << "        content.pop();" << endl;
    of << "        if (content.empty())" << endl;
    of << "        {" << endl;
    of << "            FLAG = false;" << endl;
    of << "            return ' ';" << endl;
    of << "        }" << endl;
    of << "        lineno += 1;" << endl;
    of << "        buff = content.front();" << endl;
    of << "        buffsize = buff.size();" << endl;
    of << "    }" << endl;
    of << "    return buff[pos++];" << endl;
    of << "}" << endl;

}
//int isNot(string left)
//{
//    string right = Regex[left];
//    for (auto sub : notEdge)
//    {
//        if (right.find(sub.first) != string::npos)
//            return 1;
//    }
//    return 0;
//}


// ��������: switch��������ɺ���
// ��������: ���ɱ�������е�main����switch�����ĺ���
// ��������: ofstream ����ļ���
// ��������ֵ: ��
void proSwitch(ofstream& of)
{
    tokens[1] = "START";
    for(int i = 1;i<=vecp.size()-1;i++)
    {
        int FLAG = 0;// ����if �� else if �ı�־
        int flagg = 0;// �жϵ�ǰ״̬�Ƿ���DFA�ߵı�־
        //���ѭ������״̬
        of <<"                case "<< i  << ":" << endl;
        of << "                {" << endl;
        int size = vecp[i].size();
        for (auto pair : vecp[i])
        {

            if (pair.second == -1) continue;
            flagg = 1;
            //�ڲ�ѭ������DFA��
            string left = w2l[pair.first];
            //int flag = isNot(left);
            int flag = notEdge[left];
            //int nextState = pair.second < (vecp.size() - 1)? pair.second:0; 
            int nextState = pair.second;
            string t = "";
            if (left == "ALL")// ���������ַ�
            {
                if (FLAG == 0)
                {
                    t = str_format("                    if ( pos <= buffsize )");
                    FLAG = 1;
                }
                else
                    t = str_format("                    else ( pos <= buffsize )");
            }
            else 
            {
                if (FLAG == 0)
                {
                    t = str_format("                    if (range(c, %d) == \"%s\")", flag, left.c_str());
                    FLAG = 1;
                }
                else
                    t = str_format("                    else if(range(c, %d) == \"%s\")", flag, left.c_str());
            }

            of << t << endl;

            if(i!=1) // ��״̬1ֱ��ת����Ӧ״̬
              of << "                        state = " << nextState << ";" << endl;
            else
            {
                //��״̬1ֱ�ӱ�ע����״̬��token
                of << "                    {" << endl;
                of << "                        state = " << nextState << ";" << endl;
                of << str_format("                        token = \"%s\";", tokens[nextState].c_str())<<endl;
                of << "                     }"<<endl;
            }
        }
        if (FLAG == 1)
        {
            of << "                    else" << endl;
            of << "                    {" << endl;
            of << "                        state = 0;" << endl;
            // of << str_format("                        token = \"%s\"; ",tokens[i].c_str()) << endl;
            of << "                    }" << endl ;
        }
        if (flagg == 0)
            of << "                    state = 0;" << endl;
            of << "                    break;" << endl<<endl;
            of << "                 }" << endl;
        
    }
}


// ��������: out��������ɺ���
// ��������: ���ɱ�������е�out���������ĺ���
// ��������: ofstream ����ļ���
// ��������ֵ: ��
void proOut(ofstream& of)
{
    while (!leftregexs.empty())
    {
        string name = leftregexs.front().name;
        string num = leftregexs.front().num;
        string val = leftregexs.front().val;
        leftregexs.pop();
        of << str_format("        if (token == \"%s\")", name.c_str()) << endl;
        if (val.size() != 0) //�������� ID100val ��Ҫ���tokenֵ��
        {

            of << "        {" << endl;
            //of << str_format("            //cout << \"L\" << lineno << \": \" << \"(\" << token << \", \"<< temp << \", \"<< \"%s:\"<<%s << \")\" << endl;", val.c_str(), num.c_str()) << endl;
            of << str_format("            out << \"L\" << lineno << \": \" << \"(\" << token << \", \" << %s << \", \" << \"%s\" << \": \" << temp << \")\" << endl;",  num.c_str(), val.c_str()) << endl;
            of << "        }" << endl;
        }
        else //���� comment103 ��������Ҫ���tokenֵ��
        {
            of << "        {" << endl;
            of << "            if(prelineno != lineno)" << endl;
            of << str_format("               out << \"L\" << prelineno+1 << \" - L\" << lineno  << \": \" << \"(%s, \"<<%s<<\")\" << endl;", name.c_str(),num.c_str()) << endl;
            of << "            else" << endl;
            of<<  str_format("               out << \"L\" << lineno  << \": \" << \"(%s, \"<<%s<<\")\" << endl;", name.c_str(), num.c_str()) << endl; 
            of << "        }" << endl;
        }
    }
}

// ��������: Ƕ��ѭ����������ɺ���
// ��������: ���ɱ�������е�Ƕ��ѭ�������ĺ���
// ��������: ofstream ����ļ���
// ��������ֵ: ��
void proDoubleCirs(ofstream& of)
{
    of << "    do" << endl;
    of << "    {" << endl;
    of << "        int state = 1;" << endl;
    of << "        string temp = \"\";" << endl;
    of << "        string token = \"\";" << endl;
    of << "        int prelineno = lineno;" << endl;
    of << "        while (state != 0)" << endl;
    of << "        {" << endl;
    of << "            char c = getNextChar();" << endl;
    of << "            if (FLAG == false) break;" << endl;
    of << "            switch (state) " << endl;
    of << "            {" << endl;
    //
    proSwitch(of);
    of << "            default: state = 0;" << endl;
    //
    of << "            }" << endl;
    of << "            if (state != 0)" << endl;
    of << "                temp += c;" << endl;
    of << "         }" << endl;
    of << "        if (pos > 0 && pos <= buffsize && temp.size() != 0)" << endl;
    of << "            pos -= 1;" << endl;
    of << "        if (token == \"identifier\" || token == \"op\" || token == \"ID\" || temp.size()==1)" << endl;
    of << "        {" << endl;
    of << "            string t = lookup(temp);" << endl;
    of << "            if (t != \"\")" << endl;
    of << "            {" << endl;
    of << "                token = t;" << endl;
    of << "                out << \"L\" << lineno << \": \" << token << \": \" << temp << endl;" << endl;
    of << "                continue;" << endl;
    of << "            }" << endl;
    of << "            if (token == \"op\" && pos > 0)" << endl;
    of << "            {" << endl;
    of << "                token = \"sign\";" << endl;
    of << "                out << \"L\" << lineno << \": \" << token << \" : \" << temp[0] << endl;" << endl;
    of << "                out << \"L\" << lineno << \": \" << token << \" : \" << temp[1] << endl;" << endl;
    of << "                continue;" << endl;
    of << "            }" << endl;
    of << "        }" << endl;
    //
    proOut(of);
    //

    of << "" << endl;
    of << "" << endl;
    of << "    }while (FLAG);" << endl;
    of << "" << endl;
    of << "" << endl;
}


// ��������: Դ�ļ��������ɺ���
// ��������: ���ɱ������Դ����
// ��������: ��
// ��������ֵ: ��
void geneCPP()
{

    //�������ļ�
    char cwd[256];
    _getcwd(cwd, 256);
    string out = str_format("\\%s.cpp",exename.c_str());
    string path = cwd + out;
    newLexPath = path;
    ofstream of(path, ios::out | ios::trunc);
    if (!of) {
        cerr << "�޷������ļ���" << out << endl;
    }

    //����Ԥ����
    of << "#include<iostream>" << endl;
    of << "#include<fstream>" << endl;
    of << "#include <direct.h>" << endl;
    of << "#include<queue>" << endl;
    of << "#include<string>" << endl;
    of << "using namespace std;" << endl;
    of << "int buffsize = 0;" << endl;
    of << "int pos = 0;" << endl;
    of << "int lineno = 1;" << endl;
    of << "queue<string> content;" << endl;
    of << "string buff = \"\";" << endl;
    of << "bool FLAG = true;" << endl;

    //���ɷ�Χ���� range
    prodRan(of);

    //���ɲ����ֵ亯�� lookup
    proLookup(of);


    //���ɶ�ȡԴ�ļ�����
    proRead(of);

    //���ɶ�ȡ�ַ�����
    proScan(of);


    of << "int main()" << endl << "{" << endl;
    of << "    readSource();" << endl;
    of << "    std::ofstream out(\"newLex.lex\", std::ios::out);" << endl;
    of << "    if (!out.is_open()) {" << endl;
    of << "        std::cerr << \"can't open file for outputing\" << std::endl;" << endl;
    of << "        abort();" << endl;
    of << "    }" << endl << endl << endl;
    // ���Ĵ���, ����˫��Ƕ�״����
    proDoubleCirs(of);

    of << "    out.close();" << endl;
    of << "" << endl;
    of << "" << endl;
    of << "" << endl;
    of << "" << endl;

    of << "}";
    of.close();

}

// ��������: g++������·����ȡ����
// ��������: �ڳ������е�Ŀ¼������g++���������еľ���·��
// ��������: ��
// ��������ֵ: string g++.exe�ľ���·��, û���򷵻ؿ�
string getCompPath()
{
    // ������־�ļ�, �����ȡg++����������·��
    string logname = "log.txt";
    ofstream out(logname);
    out.close();

    // �ݹ������ҳ�g++.exe���ļ���, ��д��log.txt�������
    system("for /r . %i in (*g++.exe*) do @echo %i >> log.txt");

    //��ȡlog.txt�е�·��
    ifstream in(logname);
    string path = "";
    char buff[256];
    while (!in.eof())
    {

        in.getline(buff, 256);
        string temp(buff);

        if (temp.find("g++") != string::npos)
        {
            path = temp;
            continue;
        }

    }
    return path;
}


// ��������: ��ִ�г������ɺ���
// ��������: ���ɶ�Ӧcpp�ļ��Ŀ�ִ�г���
// ��������: ��
// ��������ֵ: bool �Ƿ����ɳɹ���־
bool geneEXE()
{
    string path = getCompPath();
    if (path.size() == 0|| exename.size()==0)
        return false;

    string cmd = str_format("%s %s.cpp -o %s",path.c_str(), exename.c_str(), exename.c_str());
    system(cmd.c_str());
    return true;
}
