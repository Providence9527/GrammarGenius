#pragma once

#include <QtWidgets/QMainWindow>
#include <QtCore/QTextStream>
#include<QFileDialog>
#include<QMessageBox>
#include<QErrorMessage>
#include<QTextBlock>
#include <QStandardItemModel>
#include <QStandardItem>
#include<iostream>
#include<fstream>
#include<string>
#include"ui_SLR1.h"
#include<map>
#include<queue>
#include<set>
#include<stack>
#include <algorithm>
#include<QHeaderView>
#include<direct.h>

using namespace std;
#include "SLR1.h"
#define epsion '@'
#define Gbegin 'X'


template<typename ... Args>
static std::string str_format(const std::string& format, Args ... args)
{
    auto size_buf = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1;
    std::unique_ptr<char[]> buf(new(std::nothrow) char[size_buf]);

    if (!buf)
        return std::string("");

    std::snprintf(buf.get(), size_buf, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size_buf - 1);
}


class SLR1 : public QMainWindow
{
    Q_OBJECT

public:
    SLR1(QWidget *parent = nullptr);
    ~SLR1();
   
    struct state //非终结字符状态类
    {
        int followsize = 0; // 非终结字符对应follow集合元素个数
        bool ep = false; // 非终结字符是否推导出epsilon标志
    };
    struct dfS // 项目类
    {
        char left; // 项目左部
        string right; // 项目右部
        int pointPos=-1;//当前语句逗号位置
        char last='$';//多看一步的后继符号
        dfS() {};
        dfS(char l, string r, int p=0, char la='$')
        {
            left=l;
            right=r;
            pointPos = p;
            last = la;
        }
        bool operator==(const dfS& other) const {
            return left == other.left && right == other.right && pointPos == other.pointPos && last == other.last;
        }
        // 重载<运算符
        bool operator<(const dfS& other) const {
            return true;
        }

    };
    struct conflict  // 文法冲突信息类
    {
        int state = -1;//发生冲突的状态
        string act = "";//发生冲突的行为
        bool rs = false;//归约-移进冲突
        bool rr = false;//归约-归约冲突
        queue<dfS> prj;//冲突的项目栈
        conflict(int st,string a, bool s, bool r)
        {
            state = st;
            act = a;
            rs = s;
            rr = r;
        }
        string err() // 输出错误信息的函数
        {
            string res = str_format("In State: % d    By action : %s  \n projects are listed below:\n", state, act.c_str());
            while (!prj.empty())
            {
                dfS t = prj.front(); prj.pop();
                string d = str_format("%c -> %s\n", t.left, t.right.c_str());
                res += d;
            }
            if (rs)
            {
                res += str_format("\nReduction - Shift Collision");
            }
            else {
                res += str_format("\nReduction - Reduction Collision");
            }
            return res;
        }


    };

    struct treeNode // 语法树结点类
    {
        map<int, treeNode*> child; // 语法分析树孩子结点
        map<int, treeNode*> astChild; // 抽象语法树孩子结点
        string value=""; // 语法树结点值
        string name=""; // 语法树结点名称
        int layer = 0; // 语法树结点层数
        int num = 0; // 语法树结点序号
        QStandardItem* gui = NULL; // 语法树对应QT treeView的对象
        treeNode(string na, string val="")
        {
            name = na;
            value = val;
            layer = 0;
            num = 0;
            gui = 0;
            child.clear();

        }
        string toStr()// 结点信息输出函数
        {
            
            string s = str_format("%s(%d)[%s] has %d children", name.c_str(),num, value.c_str(), child.size());
            return s;
        }
        string str()// 结点信息输出函数
        {
            if (value.size() > 0)
                return str_format("%s(%s)", name.c_str(), value.c_str());
            else
                return str_format("%s", name.c_str(), value.c_str());
        }
        void addGuiPa(QStandardItem* parent) // GUI界面添加结点函数
        {
            gui = new QStandardItem(str().c_str());
            parent->appendRow(gui);
            return;
        }
    };

    string OP = ">@<@>=@<=@!=@<>@==@+=@-=@*=@/=@%=@++@--@&&@||@";
    int nodenum = 0;

private:
    Ui::SLR1Class ui;
    
    map<char, string> w2l;// 字母到单词的 1对1映射 
    map<string, char> l2w; // 单词到字母 1对1映射
    queue<char> alnum; // 用来进行映射的英文字母队列
    string senstr;// 保存文法规则的语句序列, 用#分割
    map<char, queue<string>> sentence;//保存语句序列
    map<string, string> prod2num;// 产生式 - 编号 1对1映射
    map<string, string> num2prod;// 产生式 - 编号 1对1映射
    map<char, set<char>> firstSet;//first集合
    map<char, set<char>> followSet;//follow集合
    char lr0dfa[200][200];//lr0dfa 邻接表
    map<char, map<int, string>> dfatable;//slr1dfa图
    map<string, int> dfaStat;//lr0dfa状态集合,即状态框
    map<string, int> slrStat;//lr1dfa状态集合,即状态框
    string fielement;//读取的左部序列, 默认第一位为语法开始符号
    set<char> action;//所有字符
    stack<conflict> conf;//slr1语法判断
    map<char, map<int, string>> slr1table;//lr1分析表
    map<char, state> s;//当前左部的状态
    map<string,int> isAst; // 判断是否为AST结点
    queue<string> idseq;//保存单词编码中的标识符的序列
    queue<string> numseq;//保存单词编码中的数字的序列
    map<char, stack<treeNode*>> nodes;// 将不同结点按照命名保存
    map<int, map<int, string>> anaTable;// 语句分析表
    
    void input();
    void processSentence(string s);
    void correct();
    bool first(char E);
    void follow();
    void slr1DFA();
    queue<dfS> genState(queue<dfS> t,int flag=0);
    void fillTable(dfS s,char c, int ever, int cur,int mod,int flag=0);
    void LR1DFA();
    bool  processStack(string input);// 语法分析栈处理函数
    bool fillAnaTab(stack<string> s, string input, string event, char c, int t);
    void processTree();
    void deleteTree();

    void init1();// 初始化Input
    void init2();// 初始化 Run视图
    void preAst(string ast, string spec);//语法树结构预备函数
    char word2letter(string s);
    void trimAll(string& s, char c);
    //bool isEnd(char c) { return int(c) < 65 || int(c) > 90; }
    bool isEnd(char c) { return fielement.find(c)==string::npos; }// 默认只有左部为非终结字符
    void printSet(set<char>s);
    string set_str(set<char> o);
    void addAll(set<char>& a, set<char>& b ,char ban);
    string dfs2str(dfS t, int flag = 0);
    string state2str(queue<dfS> s,int flag=0);
    void printState(queue<dfS> s,int flag=0);
    bool isStateExist(map<string,int> s,string t);
    bool isSlr1(queue<dfS> q,int s);
    set<char> inter(set<char> s1, set<char> s2);//判断两个集合是否有交集
    void messageBox(string s);
    bool  checkRun();// 检查Run视图的输入
    string lex2str();
    int str2int(string s);
    void geneTreeNode(char c, string prod="");
    string outspace(int i,char c);
    int shiftAst(treeNode* child, treeNode* parent, int mode);
    void asTtreeInfo(treeNode* root, int layer);
    void treeInfo(treeNode *root, int layer);
    void geneAST(treeNode* root, treeNode* astParent);
    bool isAstNode(string name);




    void showFir();
    void showFoll();
    void showJud();
    void showMap();
    void showLr0();
    void showSlr1();
    void showAnaTable();
    void showAST();
 



signals:
    void signalBackToMenu2(); //向主窗口发送返回信息的函数

private slots:
    void start();
    void open();
    void save();
    void openText();
    void choexe();
    void analyze();
    void backToMenu2();// 返回主菜单函数

};
