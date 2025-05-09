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
   
    struct state //���ս��ַ�״̬��
    {
        int followsize = 0; // ���ս��ַ���Ӧfollow����Ԫ�ظ���
        bool ep = false; // ���ս��ַ��Ƿ��Ƶ���epsilon��־
    };
    struct dfS // ��Ŀ��
    {
        char left; // ��Ŀ��
        string right; // ��Ŀ�Ҳ�
        int pointPos=-1;//��ǰ��䶺��λ��
        char last='$';//�࿴һ���ĺ�̷���
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
        // ����<�����
        bool operator<(const dfS& other) const {
            return true;
        }

    };
    struct conflict  // �ķ���ͻ��Ϣ��
    {
        int state = -1;//������ͻ��״̬
        string act = "";//������ͻ����Ϊ
        bool rs = false;//��Լ-�ƽ���ͻ
        bool rr = false;//��Լ-��Լ��ͻ
        queue<dfS> prj;//��ͻ����Ŀջ
        conflict(int st,string a, bool s, bool r)
        {
            state = st;
            act = a;
            rs = s;
            rr = r;
        }
        string err() // ���������Ϣ�ĺ���
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

    struct treeNode // �﷨�������
    {
        map<int, treeNode*> child; // �﷨���������ӽ��
        map<int, treeNode*> astChild; // �����﷨�����ӽ��
        string value=""; // �﷨�����ֵ
        string name=""; // �﷨���������
        int layer = 0; // �﷨��������
        int num = 0; // �﷨��������
        QStandardItem* gui = NULL; // �﷨����ӦQT treeView�Ķ���
        treeNode(string na, string val="")
        {
            name = na;
            value = val;
            layer = 0;
            num = 0;
            gui = 0;
            child.clear();

        }
        string toStr()// �����Ϣ�������
        {
            
            string s = str_format("%s(%d)[%s] has %d children", name.c_str(),num, value.c_str(), child.size());
            return s;
        }
        string str()// �����Ϣ�������
        {
            if (value.size() > 0)
                return str_format("%s(%s)", name.c_str(), value.c_str());
            else
                return str_format("%s", name.c_str(), value.c_str());
        }
        void addGuiPa(QStandardItem* parent) // GUI������ӽ�㺯��
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
    
    map<char, string> w2l;// ��ĸ�����ʵ� 1��1ӳ�� 
    map<string, char> l2w; // ���ʵ���ĸ 1��1ӳ��
    queue<char> alnum; // ��������ӳ���Ӣ����ĸ����
    string senstr;// �����ķ�������������, ��#�ָ�
    map<char, queue<string>> sentence;//�����������
    map<string, string> prod2num;// ����ʽ - ��� 1��1ӳ��
    map<string, string> num2prod;// ����ʽ - ��� 1��1ӳ��
    map<char, set<char>> firstSet;//first����
    map<char, set<char>> followSet;//follow����
    char lr0dfa[200][200];//lr0dfa �ڽӱ�
    map<char, map<int, string>> dfatable;//slr1dfaͼ
    map<string, int> dfaStat;//lr0dfa״̬����,��״̬��
    map<string, int> slrStat;//lr1dfa״̬����,��״̬��
    string fielement;//��ȡ��������, Ĭ�ϵ�һλΪ�﷨��ʼ����
    set<char> action;//�����ַ�
    stack<conflict> conf;//slr1�﷨�ж�
    map<char, map<int, string>> slr1table;//lr1������
    map<char, state> s;//��ǰ�󲿵�״̬
    map<string,int> isAst; // �ж��Ƿ�ΪAST���
    queue<string> idseq;//���浥�ʱ����еı�ʶ��������
    queue<string> numseq;//���浥�ʱ����е����ֵ�����
    map<char, stack<treeNode*>> nodes;// ����ͬ��㰴����������
    map<int, map<int, string>> anaTable;// ��������
    
    void input();
    void processSentence(string s);
    void correct();
    bool first(char E);
    void follow();
    void slr1DFA();
    queue<dfS> genState(queue<dfS> t,int flag=0);
    void fillTable(dfS s,char c, int ever, int cur,int mod,int flag=0);
    void LR1DFA();
    bool  processStack(string input);// �﷨����ջ������
    bool fillAnaTab(stack<string> s, string input, string event, char c, int t);
    void processTree();
    void deleteTree();

    void init1();// ��ʼ��Input
    void init2();// ��ʼ�� Run��ͼ
    void preAst(string ast, string spec);//�﷨���ṹԤ������
    char word2letter(string s);
    void trimAll(string& s, char c);
    //bool isEnd(char c) { return int(c) < 65 || int(c) > 90; }
    bool isEnd(char c) { return fielement.find(c)==string::npos; }// Ĭ��ֻ����Ϊ���ս��ַ�
    void printSet(set<char>s);
    string set_str(set<char> o);
    void addAll(set<char>& a, set<char>& b ,char ban);
    string dfs2str(dfS t, int flag = 0);
    string state2str(queue<dfS> s,int flag=0);
    void printState(queue<dfS> s,int flag=0);
    bool isStateExist(map<string,int> s,string t);
    bool isSlr1(queue<dfS> q,int s);
    set<char> inter(set<char> s1, set<char> s2);//�ж����������Ƿ��н���
    void messageBox(string s);
    bool  checkRun();// ���Run��ͼ������
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
    void signalBackToMenu2(); //�������ڷ��ͷ�����Ϣ�ĺ���

private slots:
    void start();
    void open();
    void save();
    void openText();
    void choexe();
    void analyze();
    void backToMenu2();// �������˵�����

};
