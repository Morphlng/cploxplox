#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include <optional>
#include <vector>
#include <algorithm>
#include <functional>

template <typename T>
bool isIn(const std::vector<T>& list, T elem)
{
	if (std::find(list.begin(), list.end(), elem) != list.end())
		return true;

	return false;
}

template <typename T>
bool isIn(std::vector<T>&& list, T elem)
{
	if (std::find(list.begin(), list.end(), elem) != list.end())
		return true;

	return false;
}

std::string strip(const std::string& str, char ch = ' ');

std::vector<std::string> split(const std::string& str, const std::string& regex_str);

// Use string.c_str() or string_view.data()
template <typename... Args>
std::string format(const std::string& format, Args... args)
{
	// Extra space for '\0'
	int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;

	if (size_s <= 0)
		throw std::runtime_error("Error during formatting.");

	size_t size = static_cast<size_t>(size_s);
	auto buf = std::make_unique<char[]>(size);
	std::snprintf(buf.get(), size, format.c_str(), args...);

	// We don't want the '\0' inside
	return std::string(buf.get(), buf.get() + size - 1);
}

std::optional<std::string> readfile(const std::string& file_path);

// 中文字符用两个字节存储，两个的最高位均为1
bool isChinese(char front, char back);

bool endswith(const std::string& str, const std::string& end);

// 利用RAII实现一个Finally
class Finally
{
public:
	Finally(std::function<void()> fn) :end_process(std::move(fn)) {}
	~Finally() { end_process(); }

private:
	std::function<void()> end_process;
};


#ifdef _WIN32
std::wstring s2ws(const std::string& s);
#endif