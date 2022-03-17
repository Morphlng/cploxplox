#include "Common/utils.h"
#include "Interpreter/loxlib/StandardFunctions.h"
#include "Interpreter/loxlib/NativeClass.h"
#include <iostream>
#include <chrono>
#include <algorithm>

#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define WINDOWS
#define UNICODE 1
#include <Windows.h>
#else
#define UNIX
#include <dlfcn.h>
#endif

namespace CXX
{

	NativeFunction::NativeFunction(Func callable, std::string name, int arity, int optional)
		: Callable(Callable::CallableType::FUNCTION),
		  callable(std::move(callable)), identifier(std::move(name)),
		  _arity(arity), _optional(optional)
	{
	}

	Object NativeFunction::call(Interpreter &interpreter, const std::vector<Object> &arguments)
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

		GetC::GetC() : NativeFunction([](Interpreter& interpreter, const std::vector<Object>& args)
			{
				int ch = std::cin.get();
				return Object((double)(ch)); },
			"getc", 0) {}

		Chr::Chr() : NativeFunction([](Interpreter& interpreter, const std::vector<Object>& args)
			{
				return Object(std::string(1, (char)args[0].getNumber())); },
			"chr", 1) {}

		Exit::Exit() : NativeFunction([](Interpreter& interpreter, const std::vector<Object>& args)
			{
				exit(args[0].getNumber());
				return Object();
			},
			"exit", 1) {}

		TypeOf::TypeOf() : NativeFunction([](Interpreter& interpreter, const std::vector<Object>& args)
			{
				switch (args[0].type)
				{
				case ObjectType::CALLABLE:
				{
					auto callable = args[0].getCallable();
					if (callable->type == Callable::CallableType::CLASS)
						return Object(std::string("Class"));
					else
						return Object(std::string("Function"));
				}
				case ObjectType::INSTANCE:
					return Object(args[0].getInstance()->belonging->name());

				default:
					return Object(std::string(ObjectTypeName(args[0].type)));
				} 
			},
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

		GetAttr::GetAttr() : NativeFunction([](Interpreter& interpreter, const std::vector<Object>& args)
			{
				// 不是instance，则无attribute
				if (!args[0].isInstance())
					return Object();

				InstancePtr instance = args[0].getInstance();

				// 实际上，我们应当要求第二个参数必须为字符串
				// 但是考虑到我们有封装的String和std::string两种
				// 所以用to_string
				Object attr = instance->get(args[1].to_string());
				if (attr.isNil()) {
					if (args.size() == 3)
						return args[2];
				}

				return attr; 
			},
			"getattr", 3, 1) {}

		Loadlib::Loadlib() : NativeFunction([](Interpreter& interpreter, const std::vector<Object>& args)
			{
				using getClass = NativeClass * (*)();
				using getClassName = const char* (*)();
				using getFunc = NativeFunction * (*)();
				using getFuncName = const char* (*)();

#ifdef UNIX
#define LoadLibrary(x) dlopen(x, RTLD_LAZY)
#define GetProcAddress(x, y) dlsym(x, y)
				std::string libpath = args[0].getString();
#else
				std::wstring libpath = s2ws(args[0].getString());
#endif
				auto hDll = LoadLibrary(libpath.c_str());

				if (hDll)
				{
					std::string baseFunc = "getFunc_";
					std::string baseFuncName = "getFuncName_";
					for (int i = 0;; i++)
					{
						auto funcFn = (getFunc)GetProcAddress(hDll, (baseFunc + std::to_string(i)).c_str());
						auto nameFn = (getFuncName)GetProcAddress(hDll, (baseFuncName + std::to_string(i)).c_str());
						if (!funcFn || !nameFn)
							break;

						CallablePtr callable(funcFn());
						interpreter.context->set(std::string(nameFn()), Object(std::move(callable)));
					}

					baseFunc = "getClass_";
					baseFuncName = "getClassName_";
					for (int i = 0;; i++)
					{
						auto funcFn = (getClass)GetProcAddress(hDll, (baseFunc + std::to_string(i)).c_str());
						auto nameFn = (getClassName)GetProcAddress(hDll, (baseFuncName + std::to_string(i)).c_str());
						if (!funcFn || !nameFn)
							break;

						CallablePtr callable(funcFn());
						interpreter.context->set(std::string(nameFn()), Object(std::move(callable)));
					}
				}

				return Object();
#ifdef UNIX
#undef LoadLibrary
#undef GetProcAddress
#endif
			},
			"loadlib", 1) {}
}

	NativeMethod::NativeMethod(NativeFunction::Func callable, int arity, int optional, ContextPtr env)
		: NativeFunction(std::move(callable), "", arity, optional), context(std::move(env)) {}

	Object NativeMethod::call(Interpreter &interpreter, const std::vector<Object> &arguments)
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