#include<iostream>
#include<fstream>
#include <direct.h>
#include<queue>
#include<string>
using namespace std;
int buffsize = 0;
int pos = 0;
int lineno = 1;
queue<string> content;
string buff = "";
bool FLAG = true;
string range(char c, int flag)
{
    if(c == '_' ||c == '.')
      return std::string{c};

    if(c == '/')
    {
      if(flag==2)
      return std::string{c};

    }
    if((int(c) >= 48) && (int(c) <= 57))
      return "digit";

    if((int(c) >= 97) && (int(c) <= 122))
      return "letter";

    if((int(c) >= 65) && (int(c) <= 90))
      return "letter";

    if(c == '+' ||c == '-' ||c == '*' ||c == '/' ||c == '%' ||c == '<' ||c == '=' ||c == '>' ||c == '!' ||c == ';' ||c == ',' ||c == '/' ||c == '(' ||c == ')' ||c == '[' ||c == ']' ||c == '{' ||c == '}')
      return "sgn";

    return "null";
}


string lookup(string s)
{
    if(s == "else"||s == "if"||s == "int"||s == "float"||s == "double"||s == "return"||s == "void"||s == "do"||s == "while")
      return "reservedWord";

    if(s == "_"||s == "+"||s == "-"||s == "*"||s == "/"||s == "%"||s == "<"||s == "<="||s == ">"||s == ">="||s == "=="||s == "!="||s == "="||s == ";"||s == ","||s == "("||s == ")"||s == "["||s == "]"||s == "{"||s == "}")
      return "sign";

    return "";
}


void readSource()
{
    char cwd[256];
    _getcwd(cwd, 256);
    string in = "\\in.txt";
    string ipath = cwd + in;
    ifstream tf(ipath);
    if (!tf)
    {
    std::ofstream of("in.txt", std::ios::out);
    of << "YOU NEED TO PREPARE THE INPUT" << endl;
    of.close();
    }
    ifstream f(ipath);
    if (!f) {
        cerr << "can't read file" << in << endl;
        abort();
    }
    string line = "";
    while (std::getline(f, line))
    {
        content.push(line);
    }
    while (std::getline(f, line))
    {
        content.push(line);
    }
    f.close();
    buff = content.front();
    buffsize = buff.size();
}
char getNextChar()
{
    if (pos > buffsize)
    {
        pos = 0;
        content.pop();
        if (content.empty())
        {
            FLAG = false;
            return ' ';
        }
        lineno += 1;
        buff = content.front();
        buffsize = buff.size();
    }
    return buff[pos++];
}
int main()
{
    readSource();
    std::ofstream out("newLex.lex", std::ios::out);
    if (!out.is_open()) {
        std::cerr << "can't open file for outputing" << std::endl;
        abort();
    }


    do
    {
        int state = 1;
        string temp = "";
        string token = "";
        int prelineno = lineno;
        while (state != 0)
        {
            char c = getNextChar();
            if (FLAG == false) break;
            switch (state) 
            {
                case 1:
                {
                    if (range(c, 2) == "/")
                    {
                        state = 5;
                        token = "comment";
                     }
                    else if(range(c, 0) == "_")
                    {
                        state = 2;
                        token = "identifier";
                     }
                    else if(range(c, 0) == "letter")
                    {
                        state = 2;
                        token = "identifier";
                     }
                    else if(range(c, 0) == "digit")
                    {
                        state = 3;
                        token = "number";
                     }
                    else if(range(c, 0) == "sgn")
                    {
                        state = 4;
                        token = "op";
                     }
                    else
                    {
                        state = 0;
                    }
                    break;

                 }
                case 2:
                {
                    if (range(c, 0) == "_")
                        state = 2;
                    else if(range(c, 0) == "letter")
                        state = 2;
                    else if(range(c, 0) == "digit")
                        state = 2;
                    else
                    {
                        state = 0;
                    }
                    break;

                 }
                case 3:
                {
                    if (range(c, 0) == ".")
                        state = 6;
                    else if(range(c, 0) == "OPT")
                        state = 7;
                    else if(range(c, 0) == "digit")
                        state = 3;
                    else
                    {
                        state = 0;
                    }
                    break;

                 }
                case 4:
                {
                    if (range(c, 0) == "OPT")
                        state = 7;
                    else if(range(c, 0) == "sgn")
                        state = 7;
                    else
                    {
                        state = 0;
                    }
                    break;

                 }
                case 5:
                {
                    if (range(c, 2) == "/")
                        state = 8;
                    else
                    {
                        state = 0;
                    }
                    break;

                 }
                case 6:
                {
                    if (range(c, 0) == "digit")
                        state = 9;
                    else
                    {
                        state = 0;
                    }
                    break;

                 }
                case 7:
                {
                    state = 0;
                    break;

                 }
                case 8:
                {
                    if ( pos <= buffsize )
                        state = 8;
                    else
                    {
                        state = 0;
                    }
                    break;

                 }
                case 9:
                {
                    if (range(c, 0) == "digit")
                        state = 9;
                    else
                    {
                        state = 0;
                    }
                    break;

                 }
            default: state = 0;
            }
            if (state != 0)
                temp += c;
         }
        if (pos > 0 && pos <= buffsize && temp.size() != 0)
            pos -= 1;
        if (token == "identifier" || token == "op" || token == "ID" || temp.size()==1)
        {
            string t = lookup(temp);
            if (t != "")
            {
                token = t;
                out << "L" << lineno << ": " << token << ": " << temp << endl;
                continue;
            }
            if (token == "op" && pos > 0)
            {
                token = "sign";
                out << "L" << lineno << ": " << token << " : " << temp[0] << endl;
                out << "L" << lineno << ": " << token << " : " << temp[1] << endl;
                continue;
            }
        }
        if (token == "identifier")
        {
            out << "L" << lineno << ": " << "(" << token << ", " << 100 << ", " << "name" << ": " << temp << ")" << endl;
        }
        if (token == "number")
        {
            out << "L" << lineno << ": " << "(" << token << ", " << 101 << ", " << "val" << ": " << temp << ")" << endl;
        }
        if (token == "op")
        {
            out << "L" << lineno << ": " << "(" << token << ", " << 102 << ", " << "operator" << ": " << temp << ")" << endl;
        }
        if (token == "comment")
        {
            if(prelineno != lineno)
               out << "L" << prelineno+1 << " - L" << lineno  << ": " << "(comment, "<<103<<")" << endl;
            else
               out << "L" << lineno  << ": " << "(comment, "<<103<<")" << endl;
        }


    }while (FLAG);


    out.close();




}