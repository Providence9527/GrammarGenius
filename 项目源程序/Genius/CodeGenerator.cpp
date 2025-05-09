/**
*****************************************************************************
*  Copyright (c) 2024 Liang Haifeng. All rights reserved
*
*  @file    CodeGenerator.cpp
*  @brief   本文件是词法分析核心代码文件, 包括词法分析各功能模块的具体实现
*           1. 正则表达式转NFA
*           2. NFA转DFA
*           3. DFA最小化
*           4. 源程序生成与输出
*           5. 可视化界面逻辑设置
*           
*
*  @author  梁海峰
*  @date    2024-07-29
*  @version V3.0 20240729
*

*
*----------------------------------------------------------------------------
*  @note 历史版本  修改人员    修改日期    修改内容
*  @note v1.0      梁海峰      2024-5-5    完成编写
*  @note v2.0      梁海峰      2024-5-28   完善代码, 使其适配minic
*  @note v3.0      梁海峰      2024-7-29   修改完善minic词法分析源程序生成, 使其能识别注释;同时支持静默编译cpp文件成词法分析源程序
*****************************************************************************
*/




#include "CodeGenerator.h"
#include"util.h"
#include<regex>
#include<cmath>
#define maxsize 200  //表示NFA与DFA的矩阵大小
vector<edgeNode> edge;// 边集数组
vector<vertNode> vert;//点集数组
char matrix[maxsize][maxsize];//读取边集数组为邻接矩阵方便制表
char matrix2[maxsize][maxsize];//DFA邻接矩阵
string metachar = "";// 正则表达式的元字符, 如 l(l|d)* 中, l与d都是元字符
int BEGIN, END = 0; // BEGIN 表示 NFA图开始结点序号(一般是0); END表示NFA图末尾的结点
int num3 = 0; // 最小DFA的结点个数
queue<int> Queue1;//辅助输出nfa状态转换表的队列,用于广搜
set<int> Move;//辅助转换dfa的队列，用于计算move集合
map<char, set<int>> Map;//辅助nfa状态转换表输出
map<int, int> na;//最小化dfa辅助,计算非终态与终态
vector<map<char, int>> vecp;//数组形式的状态转换表
map<int, int> isTer;// 最小化DFA判断是否为终态依据
map<int, string> tokens;//vecp每一行与对应的token的映射 
int vid = 2;//vecp状态转换表每行对应的token下标;
map<char, string> w2l;//单词到元字符的映射函数
map<string, string> Regex;//完整的正则表达式，first代表正则表达式左部， second表示右部
map<string, string> rangeRegex;//用来保存生成范围函数的正则表达式，first代表正则表达式左部， second表示右部(不包括右部含'~'的表达式与含|的表达式)
queue<string> notSGN;//由非运算符号+单个字符组成右部的正则表达式所生成的语句
map<string, string> keywdReg;//含|的范围表达式, 仅用来书写关键字
map<string, int> notEdge;//含有非运算符的DFA的边
//map<string, int> lookup;//查找保留字与运算符的映射
map<string , string> EDGE;// 状态转换表的横轴，代表DFA中的边
//map<string, Token> reToken;//正则表达式识别出来的num与type
//map<string, std::smatch> lmatch;//保存了正则表达式左部及其对应的信息，用于生成code
queue<regexLeft> leftregexs;//保存了正则表达式左部及其对应的信息，用于生成code
string exename = "";//生成的词法分析源程序文件名
string newLexPath = "";//根据正则表达式生成的cpp文件地址
string exePath = "";//选中执行exe的绝对地址
string exeRePath = "";//选中执行exe的相对地址地址
CodeGenerator::CodeGenerator(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
}

CodeGenerator::~CodeGenerator()
{}


// 函数名称: 语法规则打开文件函数
// 函数功能: 在可视化界面中打开语法规则文件并读取到文本框组件中
// 函数参数: 无
// 函数返回值 :无
void CodeGenerator::openfile()
{
    ui.inputplainTextEdit->clear();
    QFileDialog* f = new QFileDialog(this);//创建对话框实例
    f->setWindowTitle("choose the data file*.txt");// 对话框左上角提示语
    f->setNameFilter("*.txt");//筛选文件类型
    f->setViewMode(QFileDialog::Detail);

    QString filePath;
    if (f->exec() == QDialog::Accepted)// 如果确定了打开文件并成功, 就读取文件名
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

    //以下为将文件内容显示到文本框中
    while (!readStream.atEnd())
    {
        ui.inputplainTextEdit->appendPlainText(readStream.readLine());

    }
}


// 函数名称: 语法规则保存文件函数
// 函数功能: 保存在可视化界面中以文本框输入的语法规则
// 函数参数: 无
// 函数返回值 :无
void CodeGenerator::savefile()
{

    QFileDialog dlg(this);

    //获取内容的保存路径
    //QString fileName = dlg.getSaveFileName(this, tr("Save As"), "./", tr("TXT File(*.txt)"));
    QString fileName = QFileDialog::getSaveFileName(NULL, QStringLiteral("TXT SaveAs"), "newfile.txt", QString(tr("TXT File (*.txt)")),
        Q_NULLPTR, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (fileName == "")
    {
        return;
    }

    //内容保存到路径文件
    QFile file(fileName);

    //以文本方式打开
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file); //IO设备对象的地址对其进行初始化

        out << ui.inputplainTextEdit->toPlainText() << endl; //输出

        QMessageBox::warning(this, tr("Finish"), tr("Successfully save the file!"));

        file.close();
    }
    else
    {
        QMessageBox::warning(this, tr("Error"), tr("File to open file!"));
    }

}


// 函数名称: 可执行文件选择函数
// 函数功能: 以对话框的形式浏览打开编译完成并可以进行词法分析的可执行文件
// 函数参数: 无
// 函数返回值 :无
void CodeGenerator::choexe()
{

    QFileDialog* f = new QFileDialog(this);
    f->setWindowTitle("choose the data file *.exe");
    f->setNameFilter("*.exe");
    f->setViewMode(QFileDialog::Detail);

    QString filePath;//读取打开文件的绝对路径
    if (f->exec() == QDialog::Accepted)
        filePath = f->selectedFiles()[0];

    exePath = filePath.toStdString();// 保存exe文件的绝对路径
    exeRePath = exePath.substr(exePath.find_last_of('/') + 1, exePath.size());// 仅获取文件名(带后缀)
    QString qexepath = QString::fromStdString(exeRePath);
    ui.exeChotextEdit->setText(qexepath);

    //cout << exePath << endl;

    //复制一份到当前文件夹方便后续操作
    char cwd[256];
    _getcwd(cwd, 256);
    string dst(cwd);
    dst = " " + dst;
    string src = exePath;
    replace(src.begin(), src.end(), '/', '\\');
    string cmd = "copy " + src + dst;
    system(cmd.c_str());
}


// 函数名称: 源代码选择函数
// 函数功能: 以对话框的形式浏览打开需要词法分析的源代码
// 函数参数: 无
// 函数返回值 :无
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

    string srcPath = filePath.toStdString();// 保存源代码文件的绝对路径
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


// 函数名称: 单词编码函数
// 函数功能: 根据选中的可执行文件与源代码进行单词编码, 并将结果输出到文本框中
// 函数参数: 无
// 函数返回值 :无
void CodeGenerator::wordsEncode()
{
    ui.wordsEcplainTextEdit->clear();
    //1. 先将文本框中文件保存到指定的输入文件 in.txt中
    QString fileName = QString::fromStdString("in.txt");
 
    QFile file(fileName);

    //以文本方式打开
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file); //IO设备对象的地址对其进行初始化

        out << ui.readSrcplainTextEdit->toPlainText() << endl; //输出到in.txt中

        //QMessageBox::warning(this, tr("Finish"), tr("Successfully save the file!"));
        file.close();
    }
    else
    {
        QMessageBox::warning(this, tr("Error"), tr("File to open file!"));
    }


    //2. 启动exe文件
    //cout << "exec: " << exeRePath.c_str() << endl;
    system(exeRePath.c_str());

    //3. 删除exe文件, 并读取newLex.lex文件内容到右侧文本框中
    string delcmd = "del " + exeRePath;
    system(delcmd.c_str());

    string lexPath = "newLex.lex";// newLex.lex文件的相对路径
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



// 函数名称: 正则表达式输出函数
// 函数功能: 用于在DOS界面输出最终进行处理的正则表达式, 方便检查正则表达式处理是否有误
// 函数参数: 无
// 函数返回值 :无
void regexInfo()
{

    //cout << "处理完成的正则表达式如下:" << endl;
    //cout << "成图表达式:  " << endl;
    for (auto p : Regex)
    {
        //cout << p.first << "   =   " << p.second << endl;

    }

    //cout <<endl<<endl <<"范围表达式:  " << endl;
    for (auto p : rangeRegex)
    {
        //cout << p.first << "   =   " << p.second << endl;

    }


    //cout << endl << endl;
}

// 函数名称: 正则表达式处理函数
// 函数功能: 读入正则表达式, 并根据左部与右部的信息对其进行不同的处理, 最后仅返回正则表达式右部
// 函数参数: string 完整的正则表达式, 包括左部与右部
// 函数返回值 :string 正则表达式右部
string getReg(string s)
{
    int i = 0;

    int nbegin;//正则表达式开头
    while (s[i] == ' '||s[i]=='\n'||s[i]=='\t')//跳过空白, 换行符, 制表符
        i++;
    int lbegin = i;
    while (s[i] != '=')// 找到‘=’部分以便分割正则表达式左部与右部
    {
        i++;
    }
    string right = s.substr(i + 1, s.size() - i - 1);//函数返回的右部


    string  left = s.substr(lbegin, i - lbegin);//正则表达式左部
    left.erase(std::remove(left.begin(), left.end(), ' '), left.end());
    std::regex pattern = regex(R"((\D+)(\d+)(\D+)?)");//用来匹配语句中需要生成NFA的正则表达式
    std::smatch match;
    string leftname = "";

    if (std::regex_search(left, match, pattern))//处理形如ID100name 这样生成NFA的
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
    else// 处理形如digit = [0-9]这样形成范围函数的
    {
        right.erase(std::remove(right.begin(), right.end(), ' '), right.end());
        int id = right.find("~");
        if (id != string::npos)//处理含"~"非运算符的范围正则表达式
        {
            notEdge[left] = 1; //为含'~'的DFA边作一个标注
            char c = right[id + 1];
            string tCode = str_format("    if((flag == 1) && (c != '%c'))\n      return \"%s\"; \n", c,left.c_str());
            notSGN.push(tCode);
        }
        else if (right.find('|') != string::npos)//处理含"|"或运算符的范围正则表达式
        {
            keywdReg[left] = right;
        }
        else 
           rangeRegex[left] = right;
        right = "";
    }


    return right;

}

// 函数名称: 可视化界面主控逻辑函数
// 函数功能: 设置可视化界面的的执行逻辑
// 函数参数: 无
// 函数返回值 :无
void CodeGenerator::test()
{
    init();

    exename = ui.ipSrctextEdit->toPlainText().toStdString();
    //cout << "exename: " << exename << endl;
    QTextDocument* doc = ui.inputplainTextEdit->document();
    int blocks = doc->blockCount();
    QString reg = "";
    string input = "";
    //逐行读取输入的多行正则表达式并将其组织成NFA生成模块所需的格式
    for (int i = 0; i < blocks; i++)
    {
        QTextBlock textblock = doc->findBlockByNumber(i);
        string temp = textblock.text().toStdString();
        string regR = getReg(temp);//正则表达式右部
        if (regR == "") continue;
        input +=  regR+ "@"; //多行reg 通过‘@’来分割
        //input += textblock.text().toStdString();
        
    }


    //regexInfo();
    //cout << input << endl;



    //将正则表达式转成NFA图
    r2Graph(input);


    //输出NFA图
    //table1

    
    map<int, map<char, set<int>>> table1 = tableINFO1();

    ui.nfatableWidget->clear();
    int cols = table1.begin()->second.size();
    int rows = table1.size();
    ui.nfatableWidget->setColumnCount(cols + 1);
    ui.nfatableWidget->setRowCount(rows);
    QStringList title = QStringList();
    title << "";
    //cout << "NFA表头如下" << endl;
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
        ui.nfatableWidget->insertRow(rownum);//在表格中新增一行
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


    //输出DFA图
    //table2

    //将NFA图转成DFA图
    map<set<int>, map<char, set<int>>> table2 = tableINFO2();

    ui.dfatableWidget->clear();
    cols = table2.begin()->second.size();
    rows = table2.size();
    ui.dfatableWidget->setColumnCount(cols + 1);
    ui.dfatableWidget->setRowCount(rows);

    //输出表头
    w2l['?'] = "OPT";
    //cout << "DFA表头如下" << endl;
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
        ui.dfatableWidget->insertRow(rownum);//在表格中新增一行
        ui.dfatableWidget->setItem(rownum, 0, new QTableWidgetItem(set_str(it->first).c_str()));//写入行表头

        int colnum = 1;
        for (auto p : it->second)
        {
            if (p.second.size() != 0)
                ui.dfatableWidget->setItem(rownum, colnum, new QTableWidgetItem(set_str(p.second).c_str()));
            colnum += 1;
        }
        rownum += 1;
    }


    //输出最小化后的DFA图
    //table 3
    tableINFO3();
    ui.dfamtableWidget->clear();
    cols = vecp[1].size();
    rows = vecp.size();
    ui.dfamtableWidget->setColumnCount(cols + 1);
    ui.dfamtableWidget->setRowCount(rows);





    //输出表头
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
        ui.dfamtableWidget->insertRow(rownum);//在表格中新增一行
        //ui.dfamtableWidget->setItem(rownum, 0, new QTableWidgetItem(QString::number(i)));//写入行表头

        if (isTer[i] == 1)
        {
            ui.dfamtableWidget->setItem(rownum, 0, new QTableWidgetItem(str_format("(%d)",i).c_str()));//写入行表头
        }
        else
            ui.dfamtableWidget->setItem(rownum, 0, new QTableWidgetItem(QString::number(i)));//写入行表头

        
        int colnum = 1;
        for (auto p : vecp[i])
        {
            if (p.second != -1)
                ui.dfamtableWidget->setItem(rownum, colnum, new QTableWidgetItem(QString::number(p.second)));
            colnum += 1;
        }
        rownum += 1;
    }


    //生成源程序
    geneCPP();

    //将源程序内容输出
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

// 函数名称: 元字符(包括终结符与非终结符)扫描函数
// 函数功能: 扫描正则表达式涉及的所有元字符, 并将结果输入到metachar中
// 函数参数: string 表示正则表达式的字符串
// 函数返回值: 无
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
        // 逐个遍历正则表达式中的字符, 如果字符不是字符串终结符或正则表达式运算符, 转义字符/即将其
        // 视为元字符作为结果
        {
            tmp = c;
            continue;
        }
        tmp = c;
        metachar += c;
        
    }
    metachar += '#';//将NFA图的epsilon也作为元字符
}

// 函数名称: 正则表达式运算符优先顺序函数
// 函数功能: 获取正则表达式运算符的优先顺序的字符
// 函数参数: char 任意字符
// 函数返回值: int 运算符优先级
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



// 函数名称: 整型集合转字符串函数
// 函数功能: 将整型集合元素读取成string
// 函数参数:set<int> 整型集合
// 函数返回值: string  整型集合的string表示
string set_str(set<int> o)
{
    string result = "{";
    for (auto a : o)
    {
        result += to_string(a) + ", ";
    }
    result.erase(result.end() - 1); // 删掉末尾多写入的','
    result += "}";
    return result;

}

// 函数名称: DFA状态划分函数
// 函数功能: 在最小化DFA过程中, DFA原先结点会归入两个划分(包含终态结点的与其他), 此函数用于对结点集合进行划分
// 函数参数: set<int> DFA图结点构成的集合; char 元字符
// 函数返回值: set<set<int>> 结点集合的集合, 划分结果
set<set<int>> divide(set<int> s, char c)
{
    map<int, set<int>> m;
    for (auto a : s)
    {
        int flag = 0;//判断当前是否会被划分到0(终态分支)与1(非终态分支),还是3(无变化分支)的标志,为0则会划分为3
        for (int j = 0; j < num3; j++)
        {
            //cout << "(" << a << "," << j << ")" << ":" << matrix2[a][j] << endl;
            if (matrix2[a][j] == c)
            {
                flag = 1;
                m[na[j]].insert(a);
                //cout << na[j] << "支插入" << a << endl;
            }
        }
        if (flag == 0)
        {
            m[3].insert(a);
            //cout <<"无关支插入" << a << endl;
        }
    }

    set<set<int>> re;
    
    //cout << "本轮" << set_str(s) << "经过" << c << "划分如下:" << endl;

    for (auto p : m)
    {
        set<int> t = p.second;
        //cout << set_str(t) << endl;
        re.insert(p.second);
    }
    return re;
}

// 函数名称: DFA最小化处理函数
// 函数功能: 将DFA图进行最小化
// 函数参数: queue<set<int>>& q 集合结点运算队列; set<set<int>>& result 最终划分的结果
// 函数返回值:无
void process(queue<set<int>>& q, set<set<int>>& result)
{

    set<int> temp = q.front();
    //cout << endl << endl << endl;
    //cout << "出运算队列 temp:" << set_str(temp) << endl;
    q.pop();
    for (auto c : metachar)
    {
        // 逐个元字符遍历, 看当前处理的结点集合是否可划分
        if (temp.size() == 1)// 集合结点只有一个的不予划分
            break;
        set<set<int>> out = divide(temp, c); 
        if (out.size() >1 )// 结点集合的集合大于1说明可分, 新划分的若干结点集合进入运算队列
        {
            //cout << "以下分支入队：" << endl;
            for (auto t : out)
            {
                //cout << "入队:" << set_str(t) << endl;
                q.push(t);
            }
            return;
        }
    }

    result.insert(temp);
    //for (auto i : result)
        //cout << set_str(i) << "  ";

}

// 函数名称: 初始化函数
// 函数功能: 将程序运行所需的各项容器与全局变量初始化
// 函数参数: 无
// 函数返回值: 无
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

// 函数名称: NFA边运算函数
// 函数功能: 根据传入正则表达式运算符对NFA边运算栈进行运算处理, 此处的运算具体来说为作对应正则表达式的NFA图
// 函数参数: stack<edgeNode> NFA边运算栈; char 正则表达式运算符
// 函数返回值: 无
void calculate(stack<edgeNode>& mystack, char operation)
{
    edgeNode left, right, result;
    right = mystack.top(); // 弹出运算栈顶作为右操作数
    mystack.pop();
    switch (operation)
    {
        case '*':// 进行闭包运算, 仅需右操作数即可
        {
            vertNode start = vertNode(vert.size(), 0, 2); vert.push_back(start);
            vertNode end = vertNode(vert.size(), 2, 0); vert.push_back(end);//针对元字符生成对应顶点
            edgeNode e1 = edgeNode(start.nodenum, right.start); edge.push_back(e1);
            edgeNode e2 = edgeNode(right.end, right.start); edge.push_back(e2);
            edgeNode e3 = edgeNode(right.end, end.nodenum); edge.push_back(e3);
            edgeNode e4 = edgeNode(start.nodenum, end.nodenum); edge.push_back(e4);//针对元字符生成对应边结点

            result = edgeNode(start.nodenum, end.nodenum);
        }break;

        case '+':// 进行正闭包运算, 仅需右操作数即可
        {
            vertNode start = vertNode(vert.size(), 0, 2); vert.push_back(start);
            vertNode end = vertNode(vert.size(), 2, 0); vert.push_back(end);//针对元字符生成对应顶点
            edgeNode e1 = edgeNode(start.nodenum, right.start); edge.push_back(e1);
            edgeNode e2 = edgeNode(right.end, right.start); edge.push_back(e2);
            edgeNode e3 = edgeNode(right.end, end.nodenum); edge.push_back(e3);//针对元字符生成对应边结点
            result = edgeNode(start.nodenum, end.nodenum);
        }break;

        case '?':// 进行可选运算, 仅需右操作数
        {
            vertNode start = vertNode(vert.size(), 0, 2); vert.push_back(start);
            vertNode end = vertNode(vert.size(), 2, 0); vert.push_back(end);//针对元字符生成对应顶点
            edgeNode e1 = edgeNode(start.nodenum, right.start); edge.push_back(e1);
            edgeNode e2 = edgeNode(start.nodenum, end.nodenum, '?'); edge.push_back(e2);
            edgeNode e3 = edgeNode(right.end, end.nodenum); edge.push_back(e3);
            metachar += "?";
            result = edgeNode(start.nodenum, end.nodenum);
        }break;

        case '&':// 进行与运算, 需要进一步弹出NFA运算栈栈顶元素作为左操作数进行运算
        {
            left = mystack.top();
            mystack.pop();

            edgeNode e = edgeNode(left.end, right.start);
            edge.push_back(e);
            result = edgeNode(left.start, right.end);
        }break;

        case '|':// 进行或运算, 需要进一步弹出NFA运算栈栈顶元素作为左操作数进行运算
        {
            left = mystack.top();
            mystack.pop();
            vertNode start = vertNode(vert.size(), 0, 2); vert.push_back(start);
            vertNode end = vertNode(vert.size(), 2, 0); vert.push_back(end);//针对元字符生成对应顶点
            edgeNode e1 = edgeNode(start.nodenum, left.start); edge.push_back(e1);
            edgeNode e2 = edgeNode(start.nodenum, right.start); edge.push_back(e2);
            ////cout << left.start << "====" << left.end<< endl;
            edgeNode e3 = edgeNode(left.end, end.nodenum); edge.push_back(e3);
            edgeNode e4 = edgeNode(right.end, end.nodenum); edge.push_back(e4);

            result = edgeNode(start.nodenum, end.nodenum);
        }break;
    }

    mystack.push(result);// 将最终结果放回NFA运算栈
}

//函数名称： 单词转元字符函数
//函数功能： 从输入框提取单词并将单词映射到元字符，以方便进行后续正则运算
//函数参数： 若干行正则表达式组成的一个字符串
//函数返回值： 进行单词-字符映射后的一个字符串, 不再以'@'分割, 而是或运算符链接
string word2letter(string INPUT)
{
    map<string, int> exist;//判断单词是否已经出现过的函数
   
    char key = ' ';//每个英文单词在w2l映射中的关键值
    
    queue<char> alpha;// 保存26个英文字母大小写的队列
    for (int i = 97; i <= 122; i++)
        alpha.push(char(i));
    for (int i = 65; i <= 90; i++)
        alpha.push(char(i));
    string reg = "";
    int rpos = 0;
    int begin = 0;
    while ((rpos = INPUT.find('@', begin)) != string::npos && begin < INPUT.size())
    {
       // 遍历多行输入的正则表达式(这些表达式被用'@'链接起来放入了长串INPUT中)
        string input = INPUT.substr(begin, rpos - begin);// 缩小扫描范围
        string newreg = "";//进行映射之后的正则表达式
        //cout << "解析出来的语句：" << input << endl;
        begin = rpos + 1;
        int pos = 0;
        while (pos < input.size())
        // 正则表达式input中的字符
        {

            while (pos < input.size() && (input[pos] == 32 || input[pos] == 9))//跳过空格或tab
                pos += 1;

            if (isalpha(input[pos]))// 如果当前遍历到的字符是英文字母则进入单词拼接过程
            {
                int beginpos = pos;
                key = ' ';
                while (isalnum(input[pos]) || input[pos] == '_')// 字母, 数字, 下划线作为合法单词组成部分不会跳过
                {
                    pos++;
                }

                string word = input.substr(beginpos, pos - beginpos);// 根据pos来从正则表达式中切割出单词

                if ((!alpha.empty()) && exist[word] == 0)// 如果大小写英文字母集没有被映射完且当前单词没有出现过就进行映射
                {
                    key = alpha.front();
                    alpha.pop();
                    w2l[key] = word;// 将一个单词映射为一个字母
                    exist[word] = key;
                }
                newreg += char(exist[word]);
            }
            else if (input[pos] != '\0')// 遍历到非英文字母且非字符串终结符进入其他字符处理环节
            {
                char t = input[pos];
                if (getPriority(t) == 0)// 处理非正则表达式运算符
                {
                    string tt = "";
                    tt += t;
                    w2l[t] = tt;// 不作改变, 自己映射到自己
                }
                    
                newreg += t;
                pos++;
            }
        }
        reg += "(" + newreg + ")|";//当前字符串处理完毕后, 需要和后续从长串INPUT读出的其他正则表达式或运算
    }
    //cout << "以下为字母单词映射表" << endl;
    for (auto p : w2l)
    {
        //cout << "(单词)" << p.first << "------>" << "(字母)" << p.second << endl;
    }
    //cout << endl;
    reg.pop_back();//删除末尾多余的并置运算符
    //cout <<"组合之后的正则表达式" << reg << endl;
    return reg;

}


// 函数名称: 正则表达式转NFA图函数
// 函数功能: 将正则表达式转为NFA图
// 函数参数: string 多行正则表达式通过'@'衔接而成的长串
// 函数返回值: NFA图首尾结点链接而成的边结点
edgeNode r2Graph(string str)
{
    str = word2letter(str);// 将原来由单词组成的正则表达式映射成由元字符组成的正则表达式

    scanMetachar(str);// 扫描出所有元字符
    //cout << "包括元字符:" << metachar << endl;
    //计算中缀表达式,默认输入是合法的  
    stack<edgeNode> mystack_metachar;//操作数栈,暂存元字符或者它运算之后的中间结果,以边结点的形式
    stack<char> mystack_operation;//符号栈, 顶部元素一定比底部元素优先级高, 如果碰到入栈元素优先级低于栈顶元素, 执行循环弹出操作, 直到入栈元素为当前优先级最高
    int i = 0;//用来遍历正则表达式每一个字符的指针
    int size = str.size();//获取正则表达式长度
    char tmp_operation;// 保存符号栈栈顶当前栈顶运算符的临时变量
    string tmp_num;
    //vert.push_back(vertNode(vert.size(), 0, 0));
    while (i < size)
    {
        if (getPriority(str[i]) == 0)//遍历指针指向元字符的处理: 直接生成NFA边结点(边为当前元字符,点为自增序号)
        {
            if (str[i] == '@')//  '@' 为转义运算符
            {
                i += 1;
            }//处理二义性
            vertNode start = vertNode(vert.size(), 0, 1); vert.push_back(start);
            vertNode end = vertNode(vert.size(), 1, 0); vert.push_back(end);//针对元字符生成对应顶点
            edgeNode temp = edgeNode(start.nodenum, end.nodenum, str[i]);//针对元字符生成对应边结点
            edge.push_back(temp);

            //将信息登记在点集与边集数组中
            mystack_metachar.push(temp);//运算结果进栈
            if ((i + 1 < size) && ((getPriority(str[i + 1]) == 0) || str[i + 1] == '('))
            //if ((i + 1 < size) && ((metachar.find(str[i+1]!=string::npos)) || str[i + 1] == '('))
            //if ((i + 1 < size) && ((isalpha(str[i + 1]) != 0) || str[i + 1] == '('))
            //当前扫描字符及其下一个字符都为元字符, 说明进入与运算 或者下一个字符为(, 说明当前运算结果需要与()的运算结果进行与运算
            {
                str[i] = '&';// 由于正则表达式中与运算没有具体字符表示, 本程序为了方便运算自行插入一个'&'来进行与运算
                i--;

            }
            i++;

        }
        else if (str[i] == '|' || str[i] == '&' || str[i] == '*' || str[i] == '+' || str[i] == '?')// 扫描到非括号正则表达式运算符的处理
        {
            if (str[i] == '*' || str[i] == '+' || str[i] == '?')// 处理闭包与可选这些单目运算符
            {
                calculate(mystack_metachar, str[i]);// 生成相应NFA图
                //if ((i + 1 < size) && ((isalpha(str[i + 1]) != 0) || str[i + 1] == '('))
                if ((i + 1 < size) && ((getPriority(str[i + 1]) == 0) || str[i + 1] == '('))
                // 如果后续读取的字符为元字符或者左括号(代表括号运算, 结果可以视为元字符), 当前单目运算符结果需要与其进行与运算
                {
                    str[i] = '&';
                    i--;
                }
            }
            else// 处理与运算与或运算的等双目运算符
            {
                if (mystack_operation.empty())// 如果符号栈是空的就不需要比较优先级, 当前字符直接进栈
                {
                    mystack_operation.push(str[i]);
                }
                else// 如果符号栈为空需要进一步处理
                {
                    while (!mystack_operation.empty())// 不断弹出符号栈中运算符
                    {
                        tmp_operation = mystack_operation.top();// 弹出栈顶运算符
                        if (getPriority(tmp_operation) >= getPriority(str[i]))
                            // 比较当前字符与栈顶运算符的优先级, 如果栈顶运算符更优先则先以它操作NFA边运算栈
                        {
                            //计算  
                            calculate(mystack_metachar, tmp_operation);
                            mystack_operation.pop();
                        }
                        else 
                            break; // 如果当前字符优先级更高, 则入栈, 成为栈顶元素, 终结本次弹出过程
                    }
                    mystack_operation.push(str[i]);
                }

            }

            i++;
        }
        else // 处理括号运算符
        {
            if (str[i] == '(')// 左括号直接入符号栈
            {
                mystack_operation.push(str[i]);
            }
            else if(str[i]==')')
            {
                while (mystack_operation.top() != '(') //逐个弹出符号栈中元素并计算至栈顶元素为左括号
                {
                    tmp_operation = mystack_operation.top();
                    //计算  
                    calculate(mystack_metachar, tmp_operation);
                    mystack_operation.pop();
                }
                mystack_operation.pop();
                if ((i + 1 < size) && ((isalpha(str[i + 1]) != 0) || str[i + 1] == '('))
                //括号运算结果需要与后面的元字符进行与运算
                {
                    str[i] = '&';
                    i--;
                }
                ////cout << "$%%$%$%$   " << mystack_metachar.top().start << "------->" << mystack_metachar.top().end << endl;
            }
            i++;
        }

    }
    //遍历完后，若栈非空，弹出所有元素  
    while (!mystack_operation.empty()) {
        tmp_operation = mystack_operation.top();
        //计算  
        calculate(mystack_metachar, tmp_operation);
        mystack_operation.pop();
    }
    //cout << "top元素:  " << mystack_metachar.top().start << "\nend元素： " << mystack_metachar.top().end << endl;
    BEGIN = mystack_metachar.top().start;
    END = mystack_metachar.top().end;
    memset(matrix, 1, sizeof(matrix));

    // 将状态转换表处理成邻接矩阵(NFA序号为为点, 元字符为边)
    for (auto a : edge)
    {
        matrix[a.start][a.end] = a.weight;
        ////cout << "**" << matrix[a.start][a.end] << endl;
    }
    //cout << "边集数组如下:" << endl;
    graphINFO();
    return mystack_metachar.top();
}

// 函数名称: NFA图DOS窗口输入函数
// 函数功能: 以边集数组的形式在DOS窗口输出NFA图
// 函数参数: 无
// 函数返回值: 无
void graphINFO()
{
    for (auto a : edge)
    {
        //cout << a.start << "-----" << a.weight << "----->" << a.end << endl;
    }


}



// 函数名称: NFA状态转换表生成函数
// 函数功能: 将r2Graph函数处理完成NFA边集数组处理成状态转换表
// 函数参数: 无
// 函数返回值: map<int, map<char, set<int>>> NFA状态装换表
map<int, map<char, set<int>>> tableINFO1()
{
    map<int, map<char, set<int>>> final;// 保存函数返回值
    map<char, set<int>> items; // 保存 元字符 - 结点集合 一对多映射的临时容器

    int visit[maxsize];
    memset(visit, 0, sizeof(visit));

    // DOS输出检查邻接矩阵的值是否正确
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {

            //cout << matrix[i][j] << " ";
        }
        //cout << endl;
    }

    //cout << endl << endl;

    // 输出NFA状态转换表表头
    //cout << "table1:" << endl;
    //cout << "        ";
    for (auto c : metachar)
    {
        //cout << c << "       ";
    }
    //cout << endl;
    Queue1.push(BEGIN);

    //广度遍历邻接矩阵以生成状态转换表, 每一个NFA序号结点仅作为出发结点出发一次, 每一次遍历就会把序号对应的一行状态转换表补充完整
    while (!Queue1.empty())
    {
        int i = Queue1.front();
        //cout << endl << endl;
        //cout << "NFA写第" << i << "行" << endl;
        Queue1.pop();
        for (int j = 0; j <= END; j++)
        {
            if (metachar.find(matrix[i][j]) != string::npos)// 仅邻接矩阵值是元字符, 忽略memset设置的空值
            {
                Map[matrix[i][j]].insert(j);// Map 是元字符 - 结点集合 的一对多映射, 对应(类似但是不等于)状态转换表的一列
                //cout << "第" << matrix[i][j] << "列填入" << j << endl;
                ////cout << "zifuchuan: " << matrix[i][j] << "------" << Map[matrix[i][j]] << endl;
                ////cout << "当前是" << i << ",  让" << j << "进队" << endl;              
                if (visit[j] == 0)
                    Queue1.push(j);
                visit[j] = 1;
            }

        }

        //cout << i << ":       ";
       
        for (auto a : metachar) //将Map的内容放到临时容器items中, 因为每填写转换表一行就要更新一次Map
        {
            //cout << set_str(Map[a]) << "(" << a << ")" << "    ";
            items[a] = Map[a];
            Map[a].clear();
        }
        final[i] = items; // 按照访问到的NFA序号填写状态转换表对应行的信息
        items.clear();
        //cout << endl;

    }
    return final;
}


// 函数名称: move运算函数
// 函数功能: 对给定结点集合按照给定的单一元字符进行move运算(将将结点集合视为一个大结点, Move运算即将与大结点有元字符做权值的边邻接的结点挑选出来)
// 函数参数: set<int> 结点集合; char 单一元字符
// 函数返回值: 无
void move(set<int> begin, char metachar)
{
    Move.clear();// Move容器保存当前Move运算结果
    for (auto a : begin)// 逐个遍历结点集合中的结点
    {
        int id = a;
        for (int j = 0; j <= END; j++)// 
        {
            if (matrix[id][j] == metachar) // 如果当前遍历结点与j结点通过metachar邻接
            {
                Move.insert(j); // 将j放入结果容器Move
            }
        }
    }
}


// 函数名称: 闭包运算函数
// 函数功能: 对传入的结点集合进行闭包运算, 若将结点集合视为一个大结点, 则闭包运算实质是将与大结点有epsilon边邻接的结点加入大结点中
// 函数参数: set<int> 结点集合
// 函数返回值: set<int> 闭包运算结果
set<int> closure(set<int> move)
{
    set<int> temp; // 用来存放传入结点集合初值的临时容器, 因为传入的move不可修改, 因此来操作临时容器
    queue<int> queue;// 运算队列
    int visited[maxsize];// 用来标记结点是否被访问过的辅助容器, 下标表示结点, 下标对应数组值为访问标志
    for (auto a : move)// 备份结点集合
    {
        temp.insert(a);
    }
    for (auto node : move)// 遍历结点集合中的所有结点
    {
        queue.push(node);
        memset(visited, 0, sizeof(visited));
        while (!queue.empty())// 循环至运算队列为空
        {
            int i = queue.front(); queue.pop();
            visited[i] = 1;
            for (int j = 0; j <= END; j++)
            {
                if (matrix[i][j] == '#' && visited[j] == 0)// 如果当前结点i与结点j通过epsilon邻接, 则加入temp集合, 并进入运算队列
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


// 函数名称: NFA图转DFA图函数
// 函数功能: 将NFA图转为DFA图
// 函数参数: 无
// 函数返回值: 表示状态装换表的复合容器
map<set<int>, map<char, set<int>>> tableINFO2()
{
    map<set<int>, map<char, set<int>>> final; //保存最终生成DFA图状态转换表的容器
    map<char, set<int>> items; // 元字符 - 状态 的一对多映射, 在DFA状态转换表中表示某行的值(不含横纵表头)
    string t = metachar;
    metachar = "";
    for (auto c : t) // 这个循环实质其实就是将epsilon排除出元字符
    {
        if (c == '#') continue;
        metachar += c;

    }
    memset(matrix2, 1, sizeof(matrix2));
    queue<set<int>> status;//保存状态转换表的状态的运算队列队列
    map<set<int>, int> history; // DFA图一行对应唯一的一个结点集合, 也即状态, 这是一个判断状态是否出现过的容器
                               // 出现过的状态会标上其出现的顺序, 由1开始, 没出现的状态为0, 这样结点集合也被映射为一个序号
    set<int> begin; // 生成DFA图需要结点集合不断迭代, 这个就是最初的结点集合, 是为起始集合
    int flag = 0; // 判断NFA图出发点是否有邻接边为epsilon的标志

    for (auto c : edge)// 这个循环是通过遍历边集数组表示的NFA图来确定出发点是否有邻接边为epsilon
    {
        if (c.start == BEGIN && c.weight == '#')
        {
            flag = 1;

        }
    }
    if (flag == 0)
        begin.insert(BEGIN); // 如果出发点没有通过epsilon连接到其他结点, 则起始集合仅包含出发点
    else
    {
        set<int> t; t.insert(BEGIN);
        begin = closure(t); // 如果出发点有通过epsilon连接到其他结点, 则起始集合为出发点经过闭包运算后的结点集合
    }

    num3 = 0; //DFA有效状态计数器
    history[begin] = num3++;
    status.push(begin); 

    //cout << endl << endl;
    //cout << "table2:" << endl;
    //cout << "                 ";
    for (auto c : metachar) 
    {
        if (c == '#') break;
        //cout << c << "                                   ";
    }//输出表头
    //cout << endl;

    while (!status.empty()) // 循环至状态运算队列为空
    {

        set<int> temp = status.front(); // 这里弹出的temp状态就是DFA图确定存在的状态, 因为确定存在的才会进入队列
        status.pop();
        //cout << set_str(temp) << "     ";
        for (auto c : metachar) // 遍历所有元字符, 当前运算队列弹出的状态与遍历到的每一个元字符进行move运算, 接着闭包运算
                                // 这个过程就可能产生新的状态, 新的状态就会进入状态运算队列等待运算
        {

            if (c == '#') break; 
            move(temp, c); //进行move运算
            set<int> clo = closure(Move); // 进行闭包运算
            //cout << set_str(clo) << "(" << c << ")" << "       ";
            items[c] = clo; // 保存以temp作为出发状态, 经过元字符c到达的状态, 体现到状态转换表上就是填写temp状态为行号的
                            // c为列表头的状态转换表值
            if (clo.size() == 0) continue;
            if (history[clo] == 0) // 如果clo为新状态, 则用其出现的序号标记已出现, 并放入运算队列
            {
                status.push(clo);
                history[clo] = num3++;
            }
            matrix2[history[temp]][history[clo]] = c; // 更新DFA邻接矩阵, 这里状态(结点集合)映射为一个序号, 即旧DFA图多个结点
                                                      //用一个新DFA的单一结点表示
        }

        if (temp.find(END) != temp.end()) // 这个判断语句顺便把DFA最小化涉及的终态集合与非终态集合生成做了
            na[history[temp]] = 2; // 如果当前DFA结点含NFA最终结点, 则将其划入终态集合
        else
            na[history[temp]] = 1; // 否则划入非终态结点
        //cout << endl;
        final[temp] = items; // 填写状态转换表中temp行的值
        items.clear();
    }
    return final;

}


// 函数名称: DFA最小化函数
// 函数功能: 将DFA最小化
// 函数参数: 无
// 函数返回值: 无
void tableINFO3()
{
    //cout << endl << endl << endl;
    for (int i = 0; i < num3; i++) //遍历DFA图的邻接矩阵检查一下
    {
        for (int j = 0; j < num3; j++)
        {
            //cout << matrix2[i][j] << " ";
        }
        //cout << endl;
    }

    for (auto it = na.rbegin(); it != na.rend(); ++it)  // 遍历检查一下终态集合与非终态集合
    {
        //cout << it->first << "------" << it->second << endl;
    }

    set<int> N, A; // N表示非终态集合, A表示终态集合
    for (auto p : na) // 初始化终态集合与非终态集合
    {
        if (p.second == 1)
            N.insert(p.first);
        else
            A.insert(p.first);
    }

    queue<set<int>> q; // 结点集合运算队列, 
    set<set<int>> result; // 保存本次结点集合划分运算结果的容器
    q.push(N); q.push(A); // 终态集合与非终态集合作为运算队列初值, 在随后的迭代中会分出若干个最小化DFA图的状态
    while (!q.empty()) // 迭代生成最小化DFA的状态
    {
        process(q, result);
    }
    //cout << endl;
    //cout << "结点分拆为：";

    int num = 1; 
    map<int, int> m2; // 原DFA图结点 - 最小化DFA 多对一映射 

    for (auto i : result) // 这个循环是将原DFA图的所有结点映射到最小化DFA图中的结点, 相当于结点合并
    {
        if (i.size() == 0) continue;
        //cout << set_str(i) << "#  ";
        for (auto n : i)
        {
            if (A.count(n))
            {
                isTer[num] = 1;
                //cout << endl<<n << "导致" << num << "为终止结点" << endl;
            }
            m2[n] = num;
        }

        num++;
    }
    //cout << endl;

    ////cout << matrix[0][0] << "===="<<int(matrix[0][0]) << endl;

    vector<map<char, int>> vec(num);//数组形式的状态转换表, 下标表示出发状态, 值就是状态转换表一行的内容
    /*for (int i = 0; i < num; i++)
    {
        vec[0]=
    }*/
    for (int i = 0; i < num3; i++) // 通过遍历 邻接矩阵形式的原DFA 生成 最小化DFA状态转换表
    {
        for (int j = 0; j < num3; j++)
        {
            if (int(matrix2[i][j]) != 1) // memset是用1来初始化, 非1说明是存在的结点
            {
                vec[m2[i]][matrix2[i][j]] = m2[j]; // m2[i]与m2[j]都是原DFA结点映射到最小化DFA上的结点, 
                                                   // m2[i]确定最小化DFA状态转换表的行号, matrix2[i][j]确定列号, 值即m2[j]
                ////cout << m2[i] << "---" << matrix2[i][j] << "--->" << m2[j] << endl;

            }
        }
    }

    for (char c : metachar) // 这个循环只是想改变一下顺序的, 但是似乎没有用, 请忽略
    {
        for (int i = 1; i < num; i++)
        {
            if (vec[i][c] == 0)
                vec[i][c] = -1;
        }
    }


    // 输出最小化DFA检查一下
    //cout << endl << endl;
    //cout << "table3:" << endl;
    //cout << "                 ";
    for (auto c : metachar)
    {
        //cout << c << "                ";
    }//输出表头
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



// 函数名称: 范围表达式处理函数
// 函数功能: 处理[***]格式的状态表达式
// 函数参数: ofstream 文件输出流; string 表达式左部; string 表达式右部
// 函数返回值: 无

void writeRanBra(ofstream &of, string left, string right)
{
  
    //if (right.find('-') != string::npos) //处理形如[0-9] [a-z]等用连字符表示范围的表达式
    if( 3*count(right.begin(),right.end(),'-')+2 == right.size()) // 一个 '-' 对应两个终结字符, 根据这个规律判断当前表达式是否合法
    {
        //cout << "连字符 reg" << right << endl;
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
    else //处理形如 [abc]等不用连字符的表达式
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

// 函数名称: 含或范围表达式处理函数
// 函数功能: 处理形如 a|b|c 这样的范围表达式
// 函数参数: ofstream 文件输出流; string 表达式左部; string 表达式右部
// 函数返回值: 无
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

// 函数名称: range()代码生成函数
// 函数功能: 生成编译程序中的range函数代码的函数
// 函数参数: ofstream 输出文件流
// 函数返回值: 无
//
void prodRan(ofstream& of)
{
    queue<string> notReg;//含非运算符的范围表达式,会生成if语句
    queue<string> norReg;//不含非运算符的范围表达式

    string tCode = "";
    string left = "";
    string right = "";
    of << "string range(char c, int flag)" << endl << "{" << endl;;

    //首先生成处理含'~'范围正则表达式的代码
    while (!notSGN.empty())
    {
        string temp = notSGN.front(); notSGN.pop();
        of << temp<<endl;
    }


    //处理 [*-*]与[***] 类型的范围正则表达式
    for (auto ranReg : rangeRegex)
    {
        left = ranReg.first;
        right = ranReg.second;
        writeRanBra(of, left, right);

    }   


    of << "    return \"null\";"<<endl;
    of << "}" << endl<<endl<<endl;
}


// 函数名称: lookup()代码生成函数
// 函数功能: 生成编译程序中的lookup函数代码的函数
// 函数参数: ofstream 输出文件流
// 函数返回值: 无
void proLookup(ofstream &of)
{
    of << "string lookup(string s)" << endl << "{" << endl;

    //处理形如 *|**|**
    for (auto regex : keywdReg)
    {

        writeRanOr(of, regex.first, regex.second);
    }
    of << "    return \"\";" << endl;
    of << "}" << endl<<endl<<endl;
}

// 函数名称: readSource()代码生成函数
// 函数功能: 生成编译程序中的readSource函数代码的函数
// 函数参数: ofstream 输出文件流
// 函数返回值: 无
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


// 函数名称: getNextChar()代码生成函数
// 函数功能: 生成编译程序中的getNextChar函数代码的函数
// 函数参数: ofstream 输出文件流
// 函数返回值: 无
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


// 函数名称: switch代码块生成函数
// 函数功能: 生成编译程序中的main函数switch代码块的函数
// 函数参数: ofstream 输出文件流
// 函数返回值: 无
void proSwitch(ofstream& of)
{
    tokens[1] = "START";
    for(int i = 1;i<=vecp.size()-1;i++)
    {
        int FLAG = 0;// 区分if 与 else if 的标志
        int flagg = 0;// 判断当前状态是否有DFA边的标志
        //外层循环遍历状态
        of <<"                case "<< i  << ":" << endl;
        of << "                {" << endl;
        int size = vecp[i].size();
        for (auto pair : vecp[i])
        {

            if (pair.second == -1) continue;
            flagg = 1;
            //内层循环遍历DFA边
            string left = w2l[pair.first];
            //int flag = isNot(left);
            int flag = notEdge[left];
            //int nextState = pair.second < (vecp.size() - 1)? pair.second:0; 
            int nextState = pair.second;
            string t = "";
            if (left == "ALL")// 接受任意字符
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

            if(i!=1) // 非状态1直接转到对应状态
              of << "                        state = " << nextState << ";" << endl;
            else
            {
                //在状态1直接标注各种状态的token
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


// 函数名称: out代码块生成函数
// 函数功能: 生成编译程序中的out函数代码块的函数
// 函数参数: ofstream 输出文件流
// 函数返回值: 无
void proOut(ofstream& of)
{
    while (!leftregexs.empty())
    {
        string name = leftregexs.front().name;
        string num = leftregexs.front().num;
        string val = leftregexs.front().val;
        leftregexs.pop();
        of << str_format("        if (token == \"%s\")", name.c_str()) << endl;
        if (val.size() != 0) //处理形如 ID100val 需要输出token值的
        {

            of << "        {" << endl;
            //of << str_format("            //cout << \"L\" << lineno << \": \" << \"(\" << token << \", \"<< temp << \", \"<< \"%s:\"<<%s << \")\" << endl;", val.c_str(), num.c_str()) << endl;
            of << str_format("            out << \"L\" << lineno << \": \" << \"(\" << token << \", \" << %s << \", \" << \"%s\" << \": \" << temp << \")\" << endl;",  num.c_str(), val.c_str()) << endl;
            of << "        }" << endl;
        }
        else //处理 comment103 这样不需要输出token值的
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

// 函数名称: 嵌套循环代码块生成函数
// 函数功能: 生成编译程序中的嵌套循环代码块的函数
// 函数参数: ofstream 输出文件流
// 函数返回值: 无
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


// 函数名称: 源文件代码生成函数
// 函数功能: 生成编译程序源代码
// 函数参数: 无
// 函数返回值: 无
void geneCPP()
{

    //创建新文件
    char cwd[256];
    _getcwd(cwd, 256);
    string out = str_format("\\%s.cpp",exename.c_str());
    string path = cwd + out;
    newLexPath = path;
    ofstream of(path, ios::out | ios::trunc);
    if (!of) {
        cerr << "无法创建文件：" << out << endl;
    }

    //生成预声明
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

    //生成范围函数 range
    prodRan(of);

    //生成查找字典函数 lookup
    proLookup(of);


    //生成读取源文件函数
    proRead(of);

    //生成读取字符函数
    proScan(of);


    of << "int main()" << endl << "{" << endl;
    of << "    readSource();" << endl;
    of << "    std::ofstream out(\"newLex.lex\", std::ios::out);" << endl;
    of << "    if (!out.is_open()) {" << endl;
    of << "        std::cerr << \"can't open file for outputing\" << std::endl;" << endl;
    of << "        abort();" << endl;
    of << "    }" << endl << endl << endl;
    // 核心代码, 生成双重嵌套代码块
    proDoubleCirs(of);

    of << "    out.close();" << endl;
    of << "" << endl;
    of << "" << endl;
    of << "" << endl;
    of << "" << endl;

    of << "}";
    of.close();

}

// 函数名称: g++编译器路径获取函数
// 函数功能: 在程序运行的目录下搜索g++编译器运行的绝对路径
// 函数参数: 无
// 函数返回值: string g++.exe的绝对路径, 没有则返回空
string getCompPath()
{
    // 创建日志文件, 方便读取g++编译器绝对路径
    string logname = "log.txt";
    ofstream out(logname);
    out.close();

    // 递归搜索找出g++.exe的文件名, 并写入log.txt方便读出
    system("for /r . %i in (*g++.exe*) do @echo %i >> log.txt");

    //读取log.txt中的路径
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


// 函数名称: 可执行程序生成函数
// 函数功能: 生成对应cpp文件的可执行程序
// 函数参数: 无
// 函数返回值: bool 是否生成成功标志
bool geneEXE()
{
    string path = getCompPath();
    if (path.size() == 0|| exename.size()==0)
        return false;

    string cmd = str_format("%s %s.cpp -o %s",path.c_str(), exename.c_str(), exename.c_str());
    system(cmd.c_str());
    return true;
}
