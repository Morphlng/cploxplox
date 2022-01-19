#include "Common/utils.h"
#include "Interpreter/loxlib/StandardFunctions.h"
#include "Interpreter/loxlib/NativeClass.h"
#include <iostream>
#include <chrono>
#include <algorithm>

namespace CXX {

	NativeFunction::NativeFunction(Func callable, std::string name, int arity, int optional)
		: Callable(Callable::CallableType::FUNCTION),
		callable(std::move(callable)), identifier(std::move(name)),
		_arity(arity), _optional(optional)
	{
	}

	Object NativeFunction::call(Interpreter& interpreter, const std::vector<Object>& arguments)
	{
		return callable(interpreter, arguments);
	}

	int NativeFunction::arity()
	{
		return _arity;
	}

	size_t NativeFunction::required_params()
	{
		// _arity为-1的情况在interpreter中检查过
		// 此处不会溢出
		return _arity - _optional;
	}

	std::string NativeFunction::to_string()
	{
		return format("<native function %s>", identifier.c_str());
	}

	std::string NativeFunction::name()
	{
		return identifier;
	}

	std::shared_ptr<Callable> NativeFunction::bindThis(std::shared_ptr<Instance> instance)
	{
		// 只有成员函数需要bindThis
		return nullptr;
	}

	namespace standardFunctions
	{
		Clock::Clock() : NativeFunction([](Interpreter& interpreter, const std::vector<Object>& args)
			{
				using namespace std::chrono;
				double ms = static_cast<double>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
				return Object(ms);
			},
			"clock", 0) {}

		Str::Str() : NativeFunction([](Interpreter& interpreter, const std::vector<Object>& args)
			{ return Object(args[0].to_string()); },
			"str", 1) {}

		TypeOf::TypeOf() : NativeFunction([](Interpreter& interpreter, const std::vector<Object>& args)
			{ return Object(std::string(ObjectTypeName(args[0].type))); },
			"typeof", 1) {}

		Print::Print() : NativeFunction([](Interpreter& interpreter, const std::vector<Object>& args)
			{
				for (auto const& obj : args)
				{
					std::cout << obj.to_string() << " ";
				}
				std::cout << std::endl;
				return Object();
			},
			"print", -1) {}
	}

	NativeMethod::NativeMethod(NativeFunction::Func callable, int arity, int optional, ContextPtr env)
		: NativeFunction(std::move(callable), "", arity, optional), context(std::move(env))
	{}

	Object NativeMethod::call(Interpreter& interpreter, const std::vector<Object>& arguments)
	{
		ScopedContext scope(interpreter.context, context, false);

		Object result = callable(interpreter, arguments);

		return result;
	}

	std::shared_ptr<Callable> NativeMethod::bindThis(std::shared_ptr<Instance> instance)
	{
		ContextPtr newEnv = std::make_shared<Context>(context);
		newEnv->set("this", Object(instance));
		return std::make_shared<NativeMethod>(callable, _arity, _optional, newEnv);
	}

	std::string NativeMethod::to_string()
	{
		return "<native method>";
	}

}