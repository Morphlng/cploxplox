#pragma once
#include <string>

namespace CXX {

	class Container
	{
	public:
		Container(const char* name) :type(name) {}
		virtual ~Container() = default;
		virtual std::string to_string() = 0;

	public:
		const char* type;
	};

}