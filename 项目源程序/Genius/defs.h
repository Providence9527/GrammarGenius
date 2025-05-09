#ifndef DEFS_H
#include<vector>
#include<stack> 
#include<iostream>
#include<string.h>
#include<string>
#include<stdio.h>
#include<queue>
#include<map>
#include<set>
#include<algorithm>
#include<fstream>
#include <direct.h>

using namespace std;
struct vertNode
{
	int nodenum;//�����
	int in;//���
	int out;//����
	vertNode(int num, int i, int o, bool w = false)
	{
		nodenum = num;//�����
		in = i;//���
		out = o;//����
	}
};

struct edgeNode
{

	int start;//�����Ľ��
	int end;//�����Ľ��
	char weight;//Ȩ��,Ҳ��״̬ת��Ԫ�ַ�
	string state = "";//״̬����
	edgeNode(int s = 0, int e = 0, char w = '#')
	{
		start = s;
		end = e;
		weight = w;
	}
};
struct regexLeft
{
	string name = "";
	string num = "";
	string val = "";
	regexLeft(string na, string nu, string v = "")
	{
		name = na;
		num = nu;
		val = v;
	}
};
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


#endif // !DEFS_H

