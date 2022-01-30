#pragma once
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <functional>
#include <optional>

#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define Q_DECL_EXPORT __declspec(dllexport)
#define Q_DECL_IMPORT __declspec(dllimport)
#else
#define Q_DECL_EXPORT __attribute__((visibility("default")))
#define Q_DECL_IMPORT __attribute__((visibility("default")))
#endif

#ifdef DLLEXPORT
#define CXXAPI Q_DECL_EXPORT
#else
#define CXXAPI Q_DECL_IMPORT
#endif

namespace CXX
{

	// util
	class Position;
	class Error;
	// lexer
	class Token;
	// parser
	class Expr;
	class Stmt;
	// interpreter
	class Callable;
	class Instance;
	class Container;
	class Object;
	class Context;
	class Interpreter;
	// typedef
	using ContainerPtr = std::shared_ptr<Container>;
	using CallablePtr = std::shared_ptr<Callable>;
	using InstancePtr = std::shared_ptr<Instance>;
	using ContextPtr = std::shared_ptr<Context>;

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

	class Object
	{
	public:
		friend CXXAPI Object operator+(const Object &lhs, const Object &rhs);

		friend CXXAPI Object operator-(const Object &lhs, const Object &rhs);

		friend CXXAPI Object operator*(const Object &lhs, const Object &rhs);

		friend CXXAPI Object operator/(const Object &lhs, const Object &rhs);

		friend CXXAPI Object operator%(const Object &lhs, const Object &rhs);

		friend CXXAPI bool operator==(const Object &lhs, const Object &rhs);

		friend CXXAPI bool operator!=(const Object &lhs, const Object &rhs);

		friend CXXAPI bool operator>(const Object &lhs, const Object &rhs);

		friend CXXAPI bool operator>=(const Object &lhs, const Object &rhs);

		friend CXXAPI bool operator<(const Object &lhs, const Object &rhs);

		friend CXXAPI bool operator<=(const Object &lhs, const Object &rhs);

		CXXAPI Object operator-() const; // 取反
		CXXAPI Object operator!() const; // 取非

	public:
		CXXAPI Object();

		explicit CXXAPI Object(const Token &tok);

		explicit CXXAPI Object(double number);

		explicit CXXAPI Object(const std::string &str);

		explicit CXXAPI Object(bool boolean);

		explicit CXXAPI Object(CallablePtr callable);

		explicit CXXAPI Object(InstancePtr instance);

		explicit CXXAPI Object(ContainerPtr container);

		static CXXAPI Object &Nil();

		[[nodiscard]] CXXAPI bool isNumber() const;

		[[nodiscard]] CXXAPI bool isBoolean() const;

		[[nodiscard]] CXXAPI bool isString() const;

		[[nodiscard]] CXXAPI bool isCallable() const;

		[[nodiscard]] CXXAPI bool isNil() const;

		[[nodiscard]] CXXAPI bool isInstance() const;

		[[nodiscard]] CXXAPI bool isContainer() const;

		[[nodiscard]] CXXAPI double getNumber() const;

		[[nodiscard]] CXXAPI bool getBoolean() const;

		[[nodiscard]] CXXAPI std::string getString() const;

		[[nodiscard]] CXXAPI CallablePtr getCallable() const;

		[[nodiscard]] CXXAPI InstancePtr getInstance() const;

		[[nodiscard]] CXXAPI ContainerPtr getContainer() const;

		[[nodiscard]] CXXAPI std::string to_string() const;

		[[nodiscard]] CXXAPI bool is_true() const;

	public:
		ObjectType type = ObjectType::NIL;

	private:
		std::variant<bool, double, std::string, CallablePtr, InstancePtr, ContainerPtr> value;
	};

	class Container
	{
	public:
		Container(std::string name) : type(std::move(name)) {}
		virtual ~Container() = default;
		virtual CXXAPI std::string to_string() = 0;

	public:
		std::string type;
	};

	class Callable
	{
	public:
		enum class CallableType
		{
			FUNCTION,
			CLASS
		};

		Callable(CallableType type = CallableType::FUNCTION) : type(type) {}

		virtual ~Callable() = default;

		virtual CXXAPI Object call(Interpreter &interpreter, const std::vector<Object> &arguments) = 0;

		virtual CXXAPI int arity() = 0; // 参数个数

		virtual CXXAPI size_t required_params() = 0; // 必须参数个数

		virtual CXXAPI std::shared_ptr<Callable> bindThis(std::shared_ptr<Instance> instance) = 0;

		virtual CXXAPI std::string to_string() = 0;

		virtual CXXAPI std::string name() = 0;

	public:
		CallableType type;
	};

	class Class : public Callable, public std::enable_shared_from_this<Class>
	{
	public:
		explicit CXXAPI Class(std::string name, std::unordered_map<std::string, CallablePtr> methods,
							  std::optional<std::shared_ptr<Class>> superclass = std::nullopt, bool isNative = false);

		~Class() = default;

		CXXAPI Object call(Interpreter &interpreter, const std::vector<Object> &arguments) override;

		CXXAPI int arity() override;

		CXXAPI size_t required_params() override;

		CXXAPI CallablePtr bindThis(std::shared_ptr<Instance> instance) override;

		CXXAPI std::string to_string() override;

		CXXAPI std::string name() override;

		CXXAPI CallablePtr findMethods(const std::string &name);

		static std::unordered_set<std::string> reservedMethods;

	public:
		std::string className;
		std::unordered_map<std::string, CallablePtr> methods;
		std::optional<std::shared_ptr<Class>> superClass;
		bool isNative;
	};

	class Instance : public std::enable_shared_from_this<Instance>
	{
	public:
		CXXAPI explicit Instance(std::shared_ptr<Class> ClassPtr);

		CXXAPI Object get(const Token &identifier);

		CXXAPI Object get(const std::string &identifier);

		CXXAPI void set(const Token &identifier, const Object &val);

		CXXAPI void set(const std::string &identifier, const Object &val);

		CXXAPI std::string to_string();

	public:
		std::shared_ptr<Class> belonging;

		// 这里将存储该实例包含的字段，即类数据成员
		// 不同于属性(property)，因为属性既包含类成员函数，也包含类数据成员
		std::unordered_map<std::string, Object> fields;
	};

	class NativeClass : public Class
	{
	public:
		explicit NativeClass(std::string name) : Class(std::move(name), {}, std::nullopt, true) {}
		~NativeClass() = default;

	public:
		std::unordered_map<std::string, ObjectType> allowedFields;
	};

	class NativeFunction : public Callable
	{
	public:
		using Func = std::function<Object(Interpreter &, const std::vector<Object> &)>;

		CXXAPI NativeFunction(Func callable, std::string name, int arity, int optional = 0);

		CXXAPI Object call(Interpreter &interpreter, const std::vector<Object> &arguments) override;

		CXXAPI int arity() override; // 参数个数

		CXXAPI size_t required_params() override;

		CXXAPI std::shared_ptr<Callable> bindThis(std::shared_ptr<Instance> instance) override;

		CXXAPI std::string to_string() override;

		CXXAPI std::string name() override;

	public:
		static CXXAPI Object newFunction(std::string name, Func callable, int arity = -1, int optional = 0);
		static CXXAPI Object newFunction(std::string name, std::function<void()> callable);

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
		CXXAPI NativeMethod(NativeFunction::Func callable, int arity, int optional = 0, ContextPtr env = nullptr);

		CXXAPI Object call(Interpreter &interpreter, const std::vector<Object> &arguments) override;

		CXXAPI std::shared_ptr<Callable> bindThis(std::shared_ptr<Instance> instance) override;

		CXXAPI std::string to_string() override;

	public:
		ContextPtr context;
	};

	class Context
	{
	public:
		CXXAPI void set(const Token &identifier, const Object &val);

		CXXAPI void set(const std::string &key, const Object &val);

		CXXAPI void change(const Token &identifier, const Object &val);

		CXXAPI void changeAt(const Token &identifier, const Object &val, int distance);

		CXXAPI Object &get(const Token &identifier);

		CXXAPI Object &get(const std::string &identifier);

		CXXAPI Object &getAt(const Token &identifier, int distance);

		CXXAPI Object &getAt(const std::string &identifier, int distance);

	public:
		std::unordered_map<std::string, Object> variables;
	};

	class Interpreter
	{
	public:
		CXXAPI ContextPtr currContext();
		CXXAPI ContextPtr globalEnv();
	};

}