#pragma once

#include <string>
#include <vector>
#include "Interpreter/Callable.h"
#include "Interpreter/Object.h"
#include "Interpreter/Context.h"

namespace CXX {

	class FuncDeclarationStmt;
	class LambdaExpr;
	class Interpreter;

	class Function : public Callable
	{
	public:
		Function(Object& belonging, std::shared_ptr<FuncDeclarationStmt> funcDeclarationStmt, ContextPtr env);

		explicit Function(Object& belonging, std::shared_ptr<FuncDeclarationStmt> body, const std::vector<Object>& default_values, ContextPtr env);

		Object call(Interpreter& interpreter, const std::vector<Object>& arguments) override;

		int arity() override;

		size_t required_params() override;

		std::string to_string() override;

		std::string name() override;

		// 将"this"绑定到类成员函数中
		CallablePtr bindThis(InstancePtr instance) override;

	public:
		// 记录函数属于哪个类，如果不是类成员函数，则给Nil
		// 这将方便我们判断super指向的是哪个类(应为定义时所处类的父类)
		Object& belonging;

		std::shared_ptr<FuncDeclarationStmt> funcBody;

		// 默认值不需要每次call时都计算一次
		// 构造Function时我们就将其存储起来
		std::vector<Object> default_values;

		// 闭包，当前函数定义时所位于的外部环境
		// 一旦函数内调用了"自由变量"，就需要保存闭包
		ContextPtr closure;

	private:
		void init_default_values();
	};

	class LambdaFunction : public Callable
	{
	public:
		LambdaFunction(std::shared_ptr<LambdaExpr> lambdaExpr, ContextPtr env);

		Object call(Interpreter& interpreter, const std::vector<Object>& arguments) override;

		int arity() override;

		size_t required_params() override;

		std::string to_string() override;

		std::string name() override;

		// 将"this"绑定到类成员函数中
		CallablePtr bindThis(InstancePtr instance) override;

	public:
		std::shared_ptr<LambdaExpr> funcBody;

		// 默认值不需要每次call时都计算一次
		// 构造Function时我们就将其存储起来
		std::vector<Object> default_values;

		// 闭包，当前函数定义时所位于的外部环境
		// 一旦函数内调用了"自由变量"，就需要保存闭包
		// TODO: Context和Function循环引用，内存泄漏
		ContextPtr closure;

	private:
		void init_default_values();
	};

}