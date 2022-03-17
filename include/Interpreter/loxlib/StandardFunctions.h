#pragma once
#include <string>
#include <vector>
#include <functional>
#include "Interpreter/Callable.h"
#include "Common/typedefs.h"

namespace CXX {

	class Interpreter;

	class Instance;

	class NativeFunction : public Callable
	{
	public:
		using Func = std::function<Object(Interpreter&, const std::vector<Object>&)>;

		NativeFunction(Func callable, std::string name, int arity, int optional = 0);

		Object call(Interpreter& interpreter, const std::vector<Object>& arguments) override;

		int arity() override; // 参数个数

		size_t required_params() override;

		std::shared_ptr<Callable> bindThis(std::shared_ptr<Instance> instance) override;

		std::string to_string() override;

		std::string name() override;

	public:
		Func callable;
		std::string identifier;
		int _arity;	   // 总参数个数
		int _optional; // 可选参数个数
	};

	class NativeMethod : public NativeFunction
	{
		// method记录于类的哈希表中，因此这里不需要赋名
	public:
		NativeMethod(NativeFunction::Func callable, int arity, int optional = 0, ContextPtr env = nullptr);

		Object call(Interpreter& interpreter, const std::vector<Object>& arguments) override;

		std::shared_ptr<Callable> bindThis(std::shared_ptr<Instance> instance) override;

		std::string to_string() override;

	public:
		ContextPtr context;
	};

	namespace standardFunctions
	{
		class Clock : public NativeFunction
		{
		public:
			Clock();
		};

		class Str : public NativeFunction
		{
		public:
			Str();
		};
		
		class GetC :public NativeFunction
		{
		public:
			GetC();
		};

		class Chr :public NativeFunction
		{
		public:
			Chr();
		};

		class Exit: public NativeFunction
		{
		public:
			Exit();
		};

		class TypeOf : public NativeFunction
		{
		public:
			TypeOf();
		};

		class Print : public NativeFunction
		{
		public:
			Print();
		};

		class GetAttr :public NativeFunction
		{
		public:
			GetAttr();
		};

		class Loadlib :public NativeFunction
		{
		public:
			Loadlib();
		};
	}

}