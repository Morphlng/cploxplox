#pragma once
#include <map>
#include "Cxx_Core.h"

using CXX::Container;
using CXX::Interpreter;
using CXX::NativeClass;
using CXX::NativeMethod;
using CXX::Object;

// 定义一个继承自Container的元素
// 来保存字典
class Map : public Container
{
public:
	Map() : Container("Map") {}
	~Map() = default;

	void set(const Object &key, const Object &val)
	{
		m_map[key] = val;
	}

	Object &get(const Object &key)
	{
		auto it = m_map.find(key);
		if (it != m_map.end())
			return it->second;
		else
			return Object::Nil();
	}

	size_t size()
	{
		return m_map.size();
	}

	std::string to_string() override
	{
		if (m_map.empty())
			return "{}";

		std::string result = "{\n";

		for (auto &p : m_map)
		{
			result += "  " + p.first.to_string() + ": " + p.second.to_string() + "\n";
		}

		result += "}";
		return result;
	}

private:
	// Object作为std::map的key要求重载operator<
	std::map<Object, Object> m_map;
};

using MapPtr = std::shared_ptr<Map>;

bool isMap(const Object &obj);

MapPtr getMap(const Object &obj);

class Dict : public NativeClass
{
public:
	Dict();
};

#if defined(_WIN32) && defined(EXT_EXPORT)
#define EXTAPI __declspec(dllexport)
#elif defined(_WIN32)
#define EXTAPI __declspec(dllimport)
#else
#define EXTAPI
#endif

extern "C"
{
	EXTAPI NativeClass *getClass_0();
	EXTAPI const char *getClassName_0();
}