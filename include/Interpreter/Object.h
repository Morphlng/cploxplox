#pragma once

#include <memory>
#include <string>
#include <variant>
#include <optional>

namespace CXX {

	class Token;

	enum class ObjectType
	{
		NIL,
		BOOL,
		NUMBER,
		STRING,
		CALLABLE,
		INSTANCE,
		CONTAINER
	};

	const char* ObjectTypeName(ObjectType type);

	class Callable;
	class Instance;
	class Container;

	using CallablePtr = std::shared_ptr<Callable>;
	using InstancePtr = std::shared_ptr<Instance>;
	using ContainerPtr = std::shared_ptr<Container>;

	class Object
	{
	public:
		friend Object operator+(const Object& lhs, const Object& rhs);

		friend Object operator-(const Object& lhs, const Object& rhs);

		friend Object operator*(const Object& lhs, const Object& rhs);

		friend Object operator/(const Object& lhs, const Object& rhs);

		friend Object operator%(const Object& lhs, const Object& rhs);

		friend bool operator==(const Object& lhs, const Object& rhs);

		friend bool operator!=(const Object& lhs, const Object& rhs);

		friend bool operator>(const Object& lhs, const Object& rhs);

		friend bool operator>=(const Object& lhs, const Object& rhs);

		friend bool operator<(const Object& lhs, const Object& rhs);

		friend bool operator<=(const Object& lhs, const Object& rhs);

		Object operator-() const; // 取反
		Object operator!() const; // 取非

	public:
		Object();

		explicit Object(const Token& tok);

		explicit Object(double number);

		explicit Object(const std::string& str);

		explicit Object(bool boolean);

		explicit Object(CallablePtr callable);

		explicit Object(InstancePtr instance);

		explicit Object(ContainerPtr list);

		static Object& Nil();

		[[nodiscard]] bool isNumber() const;

		[[nodiscard]] bool isBoolean() const;

		[[nodiscard]] bool isString() const;

		[[nodiscard]] bool isCallable() const;

		[[nodiscard]] bool isNil() const;

		[[nodiscard]] bool isInstance() const;

		[[nodiscard]] bool isContainer() const;

		[[nodiscard]] double getNumber() const;

		[[nodiscard]] bool getBoolean() const;

		[[nodiscard]] std::string getString() const;

		[[nodiscard]] CallablePtr getCallable() const;

		[[nodiscard]] InstancePtr getInstance() const;

		[[nodiscard]] ContainerPtr getContainer() const;

		[[nodiscard]] std::string to_string() const;

		[[nodiscard]] bool is_true() const;

	public:
		ObjectType type = ObjectType::NIL;

	private:
		std::variant<bool, double, std::string, CallablePtr, InstancePtr, ContainerPtr> value;
	};

}