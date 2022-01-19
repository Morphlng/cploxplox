#pragma once
#include <unordered_map>
#include <string>
#include "Interpreter/Object.h"

namespace CXX {

	class Module
	{
	public:
		Module(std::unordered_map<std::string, Object> values);
		~Module();

		Object& get(const std::string& name);

		void set(const std::string& name, const Object& obj);

	public:
		std::unordered_map<std::string, Object> m_values;
	};

}