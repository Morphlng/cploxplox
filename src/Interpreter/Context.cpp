#include "Lexer/Token.h"
#include "Interpreter/Context.h"

namespace CXX {

	Context::Context(ContextPtr parent) : parent(std::move(parent)) {}

	Context::~Context()
	{
		variables.clear();
	}

	void Context::set(const Token& identifier, const Object& val)
	{
		variables[identifier.lexeme] = val;
	}

	void Context::set(const std::string& key, const Object& val)
	{
		variables[key] = val;
	}

	void Context::change(const Token& identifier, const Object& val)
	{
		const std::string& key = identifier.lexeme;

		auto it = variables.find(key);
		if (it != variables.end())
		{
			it->second = val;
			return;
		}

		if (parent != nullptr)
		{
			parent->change(identifier, val);
		}
	}

	void Context::changeAt(const Token& identifier, const Object& val, int distance)
	{
		Context* ptr = ancestor(distance);

		const std::string& key = identifier.lexeme;

		auto it = ptr->variables.find(key);
		if (it != ptr->variables.end())
		{
			it->second = val;
		}
	}

	Object& Context::get(const Token& identifier)
	{
		const std::string& key = identifier.lexeme;

		auto it = variables.find(key);
		if (it != variables.end())
			return it->second;

		if (parent == nullptr)
			return Object::Nil();

		return parent->get(identifier);
	}

	Object& Context::get(const std::string& key)
	{
		auto it = variables.find(key);
		if (it != variables.end())
			return it->second;

		if (parent == nullptr)
			return Object::Nil();

		return parent->get(key);
	}

	Object& Context::getAt(const Token& identifier, int distance)
	{
		Context* ptr = ancestor(distance);

		const std::string& key = identifier.lexeme;

		auto it = ptr->variables.find(key);
		if (it != ptr->variables.end())
			return it->second;
		else
			return Object::Nil();
	}

	Object& Context::getAt(const std::string& key, int distance)
	{
		Context* ptr = ancestor(distance);

		auto it = ptr->variables.find(key);
		if (it != ptr->variables.end())
			return it->second;
		else
			return Object::Nil();
	}

	Context* Context::ancestor(int distance)
	{
		Context* curr = this;
		for (int i = 0; i < distance; i++)
		{
			// 如果这里出错说明Resolver有问题
			curr = curr->parent.get();
		}

		return curr;
	}

	ScopedContext::ScopedContext(ContextPtr& origin, ContextPtr newContext, bool shouldClear) : ref(origin), previous_copy(origin), shouldClear(shouldClear)
	{
		// 这里的运作流程为：
		// 1. ref引用origin
		// 2. previous_copy拷贝了一份origin
		// 3. ref(origin)的内容替换为新的Context
		ref = std::move(newContext);
	}

	ScopedContext::~ScopedContext()
	{
		// Context和Function之间循环引用，无法自动释放
		// 因此我们需要手动释放
		if (shouldClear)
			ref->variables.clear();

		// 析构时，恢复原本的Context
		ref = previous_copy;
	}
}