#include "Common/utils.h"
#include "Lexer/Token.h"
#include "Interpreter/Object.h"
#include "Interpreter/Callable.h"
#include "Interpreter/Class.h"
#include "Interpreter/Container.h"
#include "Interpreter/RuntimeError.h"
#include "Runner.h"

namespace CXX {

	Object::Object() : type(ObjectType::NIL)
	{
	}

	Object::Object(const Token& tok)
	{
		switch (tok.type)
		{
		case TokenType::NUMBER:
			type = ObjectType::NUMBER;
			value = std::stod(tok.lexeme);
			break;

		case TokenType::TRUE:
			type = ObjectType::BOOL;
			value = true;
			break;

		case TokenType::FALSE:
			type = ObjectType::BOOL;
			value = false;
			break;

		case TokenType::STRING:
			type = ObjectType::STRING;
			value = tok.lexeme;
			break;

		case TokenType::NIL:
			type = ObjectType::NIL;
			break;

		default:
			throw RuntimeError(tok.pos_start, tok.pos_end, "Invalid token type when constructing Object");
		}
	}

	Object::Object(double number) : type(ObjectType::NUMBER), value(number)
	{
	}

	Object::Object(const std::string& str) : type(ObjectType::STRING), value(str)
	{
	}

	Object::Object(bool boolean) : type(ObjectType::BOOL), value(boolean)
	{
	}

	Object::Object(CallablePtr callable) : type(ObjectType::CALLABLE), value(std::move(callable))
	{
	}

	Object::Object(InstancePtr instance) : type(ObjectType::INSTANCE), value(std::move(instance))
	{
	}

	Object::Object(ContainerPtr list) : type(ObjectType::CONTAINER), value(std::move(list))
	{
	}

	bool Object::isNumber() const
	{
		return type == ObjectType::NUMBER;
	}

	bool Object::isBoolean() const
	{
		return type == ObjectType::BOOL;
	}

	bool Object::isString() const
	{
		return type == ObjectType::STRING;
	}

	bool Object::isCallable() const
	{
		return type == ObjectType::CALLABLE;
	}

	bool Object::isInstance() const
	{
		return type == ObjectType::INSTANCE;
	}

	bool Object::isNil() const
	{
		return type == ObjectType::NIL;
	}

	bool Object::isContainer() const
	{
		return type == ObjectType::CONTAINER;
	}

	double Object::getNumber() const
	{
		return std::get<double>(value);
	}

	bool Object::getBoolean() const
	{
		return std::get<bool>(value);
	}

	std::string Object::getString() const
	{
		return std::get<std::string>(value);
	}

	CallablePtr Object::getCallable() const
	{
		return std::get<CallablePtr>(value);
	}

	InstancePtr Object::getInstance() const
	{
		return std::get<InstancePtr>(value);
	}

	ContainerPtr Object::getContainer() const
	{
		return std::get<ContainerPtr>(value);
	}

	std::string Object::to_string() const
	{
		switch (type)
		{
		case ObjectType::NIL:
			return "nil";

		case ObjectType::BOOL:
		{
			bool val = getBoolean();
			if (val)
				return "true";
			else
				return "false";
		}
		case ObjectType::NUMBER:
		{
			double val = getNumber();
			return (long long)val == val ? std::to_string((long long)val) : std::to_string(val);
		}
		case ObjectType::STRING:
			return std::get<std::string>(value);

		case ObjectType::CALLABLE:
			return std::get<CallablePtr>(value)->to_string();

		case ObjectType::INSTANCE:
			return std::get<InstancePtr>(value)->to_string();

		case ObjectType::CONTAINER:
			return std::get<ContainerPtr>(value)->to_string();

		default:
			return "Impossible";
		}
	}

	bool Object::is_true() const
	{
		if (isBoolean())
		{
			return getBoolean();
		}
		else if (isNumber())
		{
			return getNumber() > 0;
		}

		return false;
	}

	const char* ObjectTypeName(ObjectType type)
	{
		switch (type)
		{
		case ObjectType::NIL:
			return "nil";
		case ObjectType::BOOL:
			return "bool";
		case ObjectType::NUMBER:
			return "number";
		case ObjectType::STRING:
			return "string";
		case ObjectType::CALLABLE:
			return "callable";
		case ObjectType::INSTANCE:
			return "instance";
		case ObjectType::CONTAINER:
			return "container";
		default:
			return "impossible";
		}
	}

	Object operator+(const Object& lhs, const Object& rhs)
	{
		if (lhs.isNumber() && rhs.isNumber())
		{
			return Object(lhs.getNumber() + rhs.getNumber());
		}
		else if (lhs.isString() && rhs.isString())
		{
			return Object(std::get<std::string>(lhs.value) + std::get<std::string>(rhs.value));
		}
		else if (lhs.isInstance())
		{
			std::shared_ptr<Instance> left = lhs.getInstance();
			if (auto func = left->get("__add__"); !func.isNil())
				return func.getCallable()->call(Runner::interpreter, { rhs });
			else
				throw RuntimeError(Runner::pos_start, Runner::pos_end,
					format("%s does not have overloading function __add__(other)",
						left->belonging->className.c_str()));
		}
		else if (rhs.isInstance())
		{
			return rhs + lhs; // 使用上面lhs+rhs的方法处理
		}
		else
			throw RuntimeError(Runner::pos_start, Runner::pos_end,
				format("Illegal operator '+' for operands type(%s) and type(%s)", ObjectTypeName(lhs.type),
					ObjectTypeName(rhs.type)));
	}

	Object operator-(const Object& lhs, const Object& rhs)
	{
		if (lhs.isNumber() && rhs.isNumber())
			return Object(lhs.getNumber() - rhs.getNumber());
		else if (lhs.isInstance())
		{
			std::shared_ptr<Instance> left = lhs.getInstance();
			if (auto func = left->get("__sub__"); !func.isNil())
				return func.getCallable()->call(Runner::interpreter, { rhs });
			else
				throw RuntimeError(Runner::pos_start, Runner::pos_end,
					format("%s does not have overloading function __sub__(other)",
						left->belonging->className.c_str()));
		}
		else if (rhs.isInstance())
		{
			return rhs - lhs; // 使用上面lhs+rhs的方法处理
		}
		else
			throw RuntimeError(Runner::pos_start, Runner::pos_end,
				format("Illegal operator '-' for operands type(%s) and type(%s)", ObjectTypeName(lhs.type),
					ObjectTypeName(rhs.type)));
	}

	Object operator*(const Object& lhs, const Object& rhs)
	{
		if (lhs.isNumber() && rhs.isNumber())
			return Object(lhs.getNumber() * rhs.getNumber());
		else if ((lhs.isNumber() && rhs.isString()) || (rhs.isNumber() && lhs.isString()))
		{
			std::string origin = lhs.isString() ? lhs.getString() : rhs.getString();
			std::string result = origin;

			size_t times = lhs.isNumber() ? (size_t)lhs.getNumber() : (size_t)rhs.getNumber();

			for (size_t i = 1; i < times; i++)
			{
				result += origin;
			}
			return Object(result);
		}
		else if (lhs.isInstance())
		{
			std::shared_ptr<Instance> left = lhs.getInstance();
			if (auto func = left->get("__mul__"); !func.isNil())
				return func.getCallable()->call(Runner::interpreter, { rhs });
			else
				throw RuntimeError(Runner::pos_start, Runner::pos_end,
					format("%s does not have overloading function __mul__(other)",
						left->belonging->className.c_str()));
		}
		else if (rhs.isInstance())
		{
			return rhs * lhs; // 使用上面lhs+rhs的方法处理
		}
		else
			throw RuntimeError(Runner::pos_start, Runner::pos_end,
				format("Illegal operator '*' for operands type(%s) and type(%s)", ObjectTypeName(lhs.type),
					ObjectTypeName(rhs.type)));
	}

	Object operator/(const Object& lhs, const Object& rhs)
	{
		if (lhs.isNumber() && rhs.isNumber())
		{
			double left = lhs.getNumber(), right = rhs.getNumber();
			if (right == 0.0)
			{
				throw RuntimeError(Runner::pos_start, Runner::pos_end, "Divided by 0!");
			}

			return Object(left / right);
		}
		else if (lhs.isInstance())
		{
			std::shared_ptr<Instance> left = lhs.getInstance();
			if (auto func = left->get("__div__"); !func.isNil())
				return func.getCallable()->call(Runner::interpreter, { rhs });
			else
				throw RuntimeError(Runner::pos_start, Runner::pos_end,
					format("%s does not have overloading function __div__(other)",
						left->belonging->className.c_str()));
		}
		else if (rhs.isInstance())
		{
			return rhs / lhs; // 使用上面lhs+rhs的方法处理
		}
		else
			throw RuntimeError(Runner::pos_start, Runner::pos_end,
				format("Illegal operator '/' for operands type(%s) and type(%s)", ObjectTypeName(lhs.type),
					ObjectTypeName(rhs.type)));
	}

	Object operator%(const Object& lhs, const Object& rhs)
	{
		if (lhs.isNumber() && rhs.isNumber())
		{
			long left = (long)lhs.getNumber(), right = (long)rhs.getNumber();

			return Object((double)(left % right));
		}
		else if (lhs.isInstance())
		{
			std::shared_ptr<Instance> left = lhs.getInstance();
			if (auto func = left->get("__mod__"); !func.isNil())
				return func.getCallable()->call(Runner::interpreter, { rhs });
			else
				throw RuntimeError(Runner::pos_start, Runner::pos_end,
					format("%s does not have overloading function __mod__(other)",
						left->belonging->className.c_str()));
		}
		else if (rhs.isInstance())
		{
			return rhs % lhs; // 使用上面lhs+rhs的方法处理
		}
		else
			throw RuntimeError(Runner::pos_start, Runner::pos_end,
				format("Illegal operator '%' for operands type(%s) and type(%s)", ObjectTypeName(lhs.type),
					ObjectTypeName(rhs.type)));
	}

	bool operator==(const Object& lhs, const Object& rhs)
	{
		if (lhs.type != rhs.type)
			return false;

		switch (lhs.type)
		{
		case ObjectType::NIL:
			return true;

		case ObjectType::BOOL:
			return lhs.getBoolean() == rhs.getBoolean();

		case ObjectType::NUMBER:
			return lhs.getNumber() == rhs.getNumber();

		case ObjectType::STRING:
			// 这里不使用getString，否则会进行string的拷贝
			// 严重影响性能
			return std::get<std::string>(lhs.value) == std::get<std::string>(rhs.value);

		case ObjectType::CALLABLE:
			// shared_ptr同理，拷贝会造成atomic计数器+1
			// 用时更甚
			return std::get<CallablePtr>(lhs.value) == std::get<CallablePtr>(rhs.value);

		case ObjectType::INSTANCE:
		{
			// 没有判断rhs与lhs是否为同类型实例
			// 如果有需求，用户应在__equal__中自己定义

			Object func = std::get<InstancePtr>(lhs.value)->get("__equal__");
			if (func.isNil())
				return false;
			else
			{
				Object result = func.getCallable()->call(Runner::interpreter, { rhs });
				return result.getBoolean();
			}
		}

		default:
			return false;
		}
	}

	bool operator!=(const Object& lhs, const Object& rhs)
	{
		return !(lhs == rhs);
	}

	bool operator>(const Object& lhs, const Object& rhs)
	{
		if (lhs.isNumber() && rhs.isNumber())
		{
			return lhs.getNumber() > rhs.getNumber();
		}
		else if (lhs.isString() && rhs.isString())
		{
			return std::get<std::string>(lhs.value) > std::get<std::string>(rhs.value);
		}
		else
		{
			throw RuntimeError(Runner::pos_start, Runner::pos_end,
				format("Illegal operator '>' for operands type(%s) and type(%s)", ObjectTypeName(lhs.type),
					ObjectTypeName(rhs.type)));
		}
	}

	bool operator>=(const Object& lhs, const Object& rhs)
	{
		return lhs > rhs || lhs == rhs;
	}

	bool operator<(const Object& lhs, const Object& rhs)
	{
		if (lhs.isNumber() && rhs.isNumber())
		{
			return lhs.getNumber() < rhs.getNumber();
		}
		else if (lhs.isString() && rhs.isString())
		{
			return std::get<std::string>(lhs.value) < std::get<std::string>(rhs.value);
		}
		else
		{
			throw RuntimeError(Runner::pos_start, Runner::pos_end,
				format("Illegal operator '<' for operands type(%s) and type(%s)", ObjectTypeName(lhs.type),
					ObjectTypeName(rhs.type)));
		}
	}

	bool operator<=(const Object& lhs, const Object& rhs)
	{
		return lhs < rhs || lhs == rhs;
	}

	Object Object::operator-() const
	{
		if (this->isNumber())
		{
			return Object(-getNumber());
		}

		throw RuntimeError(Runner::pos_start, Runner::pos_end, format("Illegal operator '-' for operand type(%s)", ObjectTypeName(type)));
	}

	Object Object::operator!() const
	{
		if (this->isBoolean() || this->isNumber())
		{
			return Object(!is_true());
		}

		throw RuntimeError(Runner::pos_start, Runner::pos_end, format("Illegal operator '!' for operand type(%s)", ObjectTypeName(type)));
	}

}