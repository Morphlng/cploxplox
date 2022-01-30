#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "Common/typedefs.h"
#include "Interpreter/Object.h"

namespace CXX {

	class Token;

	class Context
	{
	public:
		explicit Context(ContextPtr parent = nullptr);

		~Context();

		void set(const Token& identifier, const Object& val);

		void set(const std::string& key, const Object& val);

		void change(const Token& identifier, const Object& val);

		void changeAt(const Token& identifier, const Object& val, int distance);

		Object& get(const Token& identifier);

		Object& get(const std::string& identifier);

		Object& getAt(const Token& identifier, int distance);

		Object& getAt(const std::string& identifier, int distance);

		Context* ancestor(int distance);

	public:
		ContextPtr parent;
		std::unordered_map<std::string, Object> variables;
	};

	// 对于{}包括起来的代码，其拥有独立的(Scoped)变量环境
	class ScopedContext
	{
	public:
		ScopedContext(ContextPtr& origin, ContextPtr newContext, bool shouldClear = true);

		~ScopedContext();

	private:
		// 这里使用一个引用成员，ref指向Interpreter原本的Context
		ContextPtr& ref;

		// copy一份原本的Context内容
		ContextPtr previous_copy;

		bool shouldClear;
	};

}