#include "Common/utils.h"
#include <regex>
#include <fstream>
#include <algorithm>

std::string strip(const std::string &str, char ch)
{
	// 除去str两端的ch字符
	int i = 0;
	size_t length = str.size();

	while (i < length && str[i] == ch) // 头部ch字符个数是i
		i++;
	if (i >= length)
		return "";

	int j = str.size() - 1;
	while (j >= 0 && str[j] == ch) // 尾部ch字符个数
		j--;
	return str.substr(i, j + 1 - i);
}

std::vector<std::string> split(const std::string &str, const std::string &regex_str)
{
	std::regex regexz(regex_str);
	std::vector<std::string> list(std::sregex_token_iterator(str.begin(), str.end(), regexz, -1), std::sregex_token_iterator());
	return list;
}

std::optional<std::string> readfile(const std::string &file_path)
{
	std::ifstream ifs;
	ifs.exceptions(std::ifstream::failbit);
	try
	{
		ifs.open(file_path);
		std::string text((std::istreambuf_iterator<char>(ifs)),
						 (std::istreambuf_iterator<char>()));
		ifs.close();
		return text;
	}
	catch (const std::exception &e)
	{
		return std::nullopt;
	}
}

bool isChinese(char front, char back)
{
	if ((front & 0x80) && (back & 0x80))
		return true;

	return false;
}

bool endswith(const std::string &str, const std::string &end)
{
	int srclen = str.size();
	int endlen = end.size();
	if (srclen >= endlen)
	{
		std::string temp = str.substr(srclen - endlen, endlen);
		if (temp == end)
			return true;
	}

	return false;
}

#ifdef _WIN32
#include <Windows.h>
std::wstring s2ws(const std::string &s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t *buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}
#endif