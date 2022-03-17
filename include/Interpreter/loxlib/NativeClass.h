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
		static std::shared_ptr<String> getSingleton();

		static InstancePtr instantiate(const std::string& str);
		static InstancePtr instantiate(const Object& obj);
	};

	class List : public NativeClass
	{
	public:
		List();
		static std::shared_ptr<List> getSingleton();

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

	// 这个接口专门用来分出一个Object属于的类
	struct Classifier
	{
		static std::string className(const Object& val);
		static bool belongClass(const Object& obj, const char* expect);
	};

}