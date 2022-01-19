#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "Interpreter/Object.h"
#include "Interpreter/Callable.h"

namespace CXX {

	class Interpreter;

	class Instance;

	class Class : public Callable, public std::enable_shared_from_this<Class>
	{
	public:
		explicit Class(std::string name, std::unordered_map<std::string, CallablePtr> methods,
			std::optional<std::shared_ptr<Class>> superclass = std::nullopt, bool isNative = false);

		~Class() = default;

		Object call(Interpreter& interpreter, const std::vector<Object>& arguments) override;

		int arity() override;

		size_t required_params() override;

		CallablePtr bindThis(std::shared_ptr<Instance> instance) override;

		std::string to_string() override;

		std::string name() override;

		CallablePtr findMethods(const std::string& name);

		static std::unordered_set<std::string> reservedMethods;

	public:
		std::string className;
		std::unordered_map<std::string, CallablePtr> methods;
		std::optional<std::shared_ptr<Class>> superClass;
		bool isNative;
	};

	class Token;

	class Instance : public std::enable_shared_from_this<Instance>
	{
	public:
		explicit Instance(std::shared_ptr<Class> ClassPtr);

		~Instance();

		Object get(const Token& identifier);

		Object get(const std::string& identifier);

		void set(const Token& identifier, const Object& val);

		void set(const std::string& identifier, const Object& val);

		std::string to_string();

	public:
		std::shared_ptr<Class> belonging;

		// 这里将存储该实例包含的字段，即类数据成员
		// 不同于属性(property)，因为属性既包含类成员函数，也包含类数据成员
		std::unordered_map<std::string, Object> fields;
	};

}