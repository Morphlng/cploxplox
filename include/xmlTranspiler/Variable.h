#pragma once
#include <string>
#include <vector>

struct Function
{
	std::string name;
	std::vector<std::string> arg_names;
	bool hasRet;
};