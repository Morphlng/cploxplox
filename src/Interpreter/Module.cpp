#include "Interpreter/Module.h"

namespace CXX {

	Module::Module(std::unordered_map<std::string, Object> values) : m_values(std::move(values)) {}

	Module::~Module()
	{
		m_values.clear();
	}

	Object& Module::get(const std::string& name)
	{
		auto it = m_values.find(name);
		if (it != m_values.end())
			return it->second;

		return Object::Nil();
	}

	void Module::set(const std::string& name, const Object& obj)
	{
		m_values.emplace(name, obj);
	}

}