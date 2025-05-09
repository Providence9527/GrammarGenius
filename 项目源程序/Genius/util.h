#ifndef UTIL_H
#define UTIL_H
#include"defs.h"

using namespace std;
string input();
int getPriority(char op);
void calculate(stack<edgeNode>& mystack, char operation);
edgeNode r2Graph(string str);
void graphINFO();
map<int, map<char, set<int>>> tableINFO1();
void move(set<int>begin, char metachar);
set<int> closure(set<int> move);
map<set<int>, map<char, set<int>>> tableINFO2();
string set_str(set<int> o);
void tableINFO3();
void toCode();
void prodRan(ofstream& of);
void geneCPP();
bool geneEXE();
void init();
#endif // !UTIL_H


