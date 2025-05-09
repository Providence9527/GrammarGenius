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
	int nodenum;//结点标号
	int in;//入度
	int out;//出度
	vertNode(int num, int i, int o, bool w = false)
	{
		nodenum = num;//结点标号
		in = i;//入度
		out = o;//出度
	}
};

struct edgeNode
{

	int start;//出发的结点
	int end;//结束的结点
	char weight;//权重,也即状态转换元字符
	string state = "";//状态集合
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

