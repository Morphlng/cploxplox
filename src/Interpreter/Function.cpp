#include "Common/utils.h"
#include "Interpreter/Function.h"
#include "Interpreter/Interpreter.h"
#include "Runner.h"

namespace CXX
{

	Function::Function(Object &belonging, std::shared_ptr<FuncDeclarationStmt> funcDeclarationStmt, ContextPtr env)
		: belonging(belonging), funcBody(std::move(funcDeclarationStmt)), closure(std::move(env))
	{
		init_default_values();
	}

	Function::Function(Object &belonging, std::shared_ptr<FuncDeclarationStmt> body, const std::vector<Object> &default_values, ContextPtr env)
		: belonging(belonging), funcBody(std::move(body)), default_values(default_values), closure(std::move(env)) {}

	Object Function::call(Interpreter &interpreter, const std::vector<Object> &arguments)
	{
		ContextPtr newEnv = std::make_shared<Context>(closure);

		size_t i, arg_size = arguments.size();
		int _arity = arity();

		for (i = 0; i < arg_size; i++)
			newEnv->set(funcBody->params[i], arguments[i]);

		// 调用call之前已经确保了参数个数在合法范围内
		// 这里只需要补全
		if (arg_size < _arity)
		{
			for (size_t count = _arity - arg_size; count > 0; count--)
				newEnv->set(funcBody->params[i++], *(default_values.end() - count));
		}

		ScopedContext scope(interpreter.context, std::move(newEnv), false);

		for (auto &stmt : funcBody->body)
		{
			interpreter.execute(stmt.get());

			if (interpreter.m_returns)
				break;
		}

		return interpreter.m_returns ? interpreter.getReturn() : Object();
	}

	int Function::arity()
	{
		return funcBody->params.size();
	}

	size_t Function::required_params()
	{
		return funcBody->params.size() - this->default_values.size();
	}

	std::string Function::to_string()
	{
		return format("<function %s>", name().c_str());
	}

	std::string Function::name()
	{
		return funcBody->name.lexeme;
	}

	CallablePtr Function::bindThis(InstancePtr instance)
	{
		ContextPtr newEnv = std::make_shared<Context>(closure);
		newEnv->set("this", Object(instance));

		// default_values一并传，不需要再计算一次
		return std::make_shared<Function>(belonging, funcBody, default_values, std::move(newEnv));
	}

	void Function::init_default_values()
	{
		// 该函数仅在首次构造Function时调用
		if (!funcBody->default_values.empty())
		{
			for (auto &val : funcBody->default_values)
			{
				this->default_values.push_back(Runner::interpreter.interpret(val.get()));
			}
		}
	}

	LambdaFunction::LambdaFunction(std::shared_ptr<LambdaExpr> lambdaExpr, ContextPtr env)
		: funcBody(std::move(lambdaExpr)), closure(std::move(env))
	{
		init_default_values();
	}

	Object LambdaFunction::call(Interpreter &interpreter, const std::vector<Object> &arguments)
	{
		ContextPtr newEnv = std::make_shared<Context>(closure);

		size_t i, arg_size = arguments.size();
		int _arity = arity();

		for (i = 0; i < arg_size; i++)
		{
			newEnv->set(funcBody->params[i], arguments[i]);
		}

		// 调用call之前已经确保了参数个数在合法范围内
		// 这里只需要补全
		if (arg_size < _arity)
		{
			for (size_t count = _arity - arg_size; count > 0; count--)
			{
				newEnv->set(funcBody->params[i++], *(default_values.end() - count));
			}
		}

		ScopedContext scope(interpreter.context, std::move(newEnv), false);

		for (auto &stmt : funcBody->body)
		{
			interpreter.execute(stmt.get());

			if (interpreter.m_returns)
			{
				break;
			}
		}

		return interpreter.m_returns ? interpreter.getReturn() : Object();
	}

	int LambdaFunction::arity()
	{
		return funcBody->params.size();
	}

	size_t LambdaFunction::required_params()
	{
		return funcBody->params.size() - this->default_values.size();
	}

	std::string LambdaFunction::to_string()
	{
		return "<anonymous function>";
	}

	std::string LambdaFunction::name()
	{
		return "anonymous";
	}

	CallablePtr LambdaFunction::bindThis(InstancePtr instance)
	{
		return nullptr;
	}

	void LambdaFunction::init_default_values()
	{
		// 该函数仅在首次构造Function时调用
		if (!funcBody->default_values.empty())
		{
			for (auto &val : funcBody->default_values)
			{
				this->default_values.push_back(Runner::interpreter.interpret(val.get()));
			}
		}
	}

}