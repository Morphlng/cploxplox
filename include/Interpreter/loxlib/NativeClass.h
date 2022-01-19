#pragma once

#include <string>
#include "Interpreter/Interpreter.h"
#include "Interpreter/Class.h"
#include "Interpreter/loxlib/StandardFunctions.h"

namespace CXX {

	class NativeClass : public Class
	{
	public:
		explicit NativeClass(std::string name) : Class(std::move(name), {}, std::nullopt, true) {}
		~NativeClass() = default;

	public:
		std::unordered_map<std::string, ObjectType> allowedFields;
	};

	class String : public NativeClass
	{
	public:
		String();
		InstancePtr instantiate(const std::string& str);
		InstancePtr instantiate(const Object& obj);
	};

	class List : public NativeClass
	{
	public:
		List();
		static InstancePtr instantiate(std::vector<Object> items);
	};

	class Mathematics : public NativeClass
	{
		// Mathematics不允许用户修改其中的变量
		// 常量定义在编译时赋予
		// 唯一实例Math
	public:
		Mathematics();
		static InstancePtr instantiate();
	};

	// 这个类专门用来分出一个Object属于的类
	struct Classifier
	{
		enum class ClassType
		{
			STRING,		 // 内置类String
			LIST,		 // 内置类List
			MATHEMATICS, // 内置类Mathematics
			NONE,		 // 非类
			CUSTOM		 // 用户自定类
		};

		static ClassType classify(const Object& val);
		static bool belongClass(const Object& val, ClassType expect);
	};

}