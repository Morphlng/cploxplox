#include "Interpreter/Class.h"
#include "Interpreter/Function.h"
#include "Interpreter/loxlib/NativeClass.h"
#include "Common/utils.h"
#include "Runner.h"

namespace CXX
{

	Class::Class(std::string name, std::unordered_map<std::string, CallablePtr> methods,
				 std::optional<std::shared_ptr<Class>> superclass, bool isNative) : Callable(CallableType::CLASS), className(std::move(name)),
																					methods(std::move(methods)), superClass(std::move(superclass)),
																					isNative(isNative) {}

	Object Class::call(Interpreter &interpreter, const std::vector<Object> &arguments)
	{
		InstancePtr instance = std::make_shared<Instance>(shared_from_this());
		if (auto initializer = findMethods("init"))
		{
			initializer->bindThis(instance)->call(interpreter, arguments);
		}

		return Object(instance);
	}

	int Class::arity()
	{
		if (auto initializer = findMethods("init"))
		{
			return initializer->arity();
		}

		return 0;
	}

	size_t Class::required_params()
	{
		if (auto initializer = findMethods("init"))
		{
			return initializer->required_params();
		}

		return 0;
	}

	std::string Class::to_string()
	{
		return format("<Class %s>", name().c_str());
	}

	std::string Class::name()
	{
		return className;
	}

	CallablePtr Class::findMethods(const std::string &name)
	{
		// reservedMethods是每个类专有的，不要去父类中寻找
		bool look_in_this = false;
		if (reservedMethods.find(name) != reservedMethods.end())
		{
			look_in_this = true;
		}

		if (methods.find(name) != methods.end())
			return methods[name];

		if (!look_in_this && superClass)
		{
			return superClass.value()->findMethods(name);
		}

		return nullptr;
	}

	CallablePtr Class::bindThis(std::shared_ptr<Instance> instance)
	{
		return shared_from_this();
	}

	Instance::Instance(std::shared_ptr<Class> ClassPtr) : belonging(std::move(ClassPtr))
	{
	}

	Instance::~Instance()
	{
		Class *ptr = belonging.get();
		// 不能在析构函数中调用shared_from_this
		// 但是调用函数需要绑定实例，所以我们手动shared
		// 注意自定义析构函数，不要delete this
		InstancePtr instance(this, [](Instance *ptr)
							 { ptr = nullptr; });

		// 由子类到父类依次析构
		do
		{
			if (auto destructor = ptr->findMethods("__del__"))
			{
				destructor->bindThis(instance)->call(Runner::interpreter, {});
			}

			if (ptr->superClass)
			{
				ptr = ptr->superClass.value().get();
			}
			else
			{
				ptr = nullptr;
			}

		} while (ptr);

		belonging.reset();
		fields.clear();
	}

	Object Instance::get(const Token &identifier)
	{
		const std::string &key = identifier.lexeme;

		auto it = fields.find(key);
		if (it != fields.end())
			return it->second;

		if (auto method = belonging->findMethods(key))
		{
			CallablePtr bindFunc(method->bindThis(shared_from_this()));
			return Object(bindFunc);
		}

		return Object();
	}

	Object Instance::get(const std::string &key)
	{
		auto it = fields.find(key);
		if (it != fields.end())
			return it->second;

		if (auto method = belonging->findMethods(key))
		{
			CallablePtr bindFunc(method->bindThis(shared_from_this()));
			return Object(bindFunc);
		}

		return Object();
	}

	void Instance::set(const Token &identifier, const Object &val)
	{
		if (belonging->isNative)
		{
			auto ptr = std::static_pointer_cast<NativeClass>(belonging);

			// 内部类只允许设定固定的几个字段(因为自定义的没意义)，并且有类型要求
			// 我们也不会报错，只是不会真的设定
			if (ptr->allowedFields.find(identifier.lexeme) == ptr->allowedFields.end())
				return;
			else if (ptr->allowedFields[identifier.lexeme] != val.type)
				return;
		}

		fields[identifier.lexeme] = val;
	}

	void Instance::set(const std::string &identifier, const Object &val)
	{
		if (belonging->isNative)
		{
			auto ptr = std::static_pointer_cast<NativeClass>(belonging);

			// 内部类只允许设定固定的几个字段(因为自定义的没意义)，并且有类型要求
			// 我们也不会报错，只是不会真的设定
			if (ptr->allowedFields.find(identifier) == ptr->allowedFields.end())
				return;
			else if (ptr->allowedFields[identifier] != val.type)
				return;
		}

		fields[identifier] = val;
	}

	std::string Instance::to_string()
	{
		// 如果有重载的表示方法，则调用
		if (auto printer = get("__repr__"); !printer.isNil())
		{
			auto task = Runner::interpreter.toggleRepl();

			Object result = printer.getCallable()->call(Runner::interpreter, {});
			return result.getString();
		}

		// 默认方法将显示实例的所属类与当前拥有字段
		std::string result = format("<Instance of %s>", belonging->name().c_str());

		if (!fields.empty())
		{
			result += "\n{\n";
			for (auto &[name, prop] : fields)
			{
				result += format("  %s: %s\n", name.c_str(), prop.to_string().c_str());
			}
			result.push_back('}');
		}

		return result;
	}

	std::unordered_set<std::string> Class::reservedMethods = {
		"__add__",	 // +
		"__sub__",	 // -
		"__mul__",	 // *
		"__div__",	 // /
		"__mod__",	 // %
		"__equal__", // ==
		"__repr__",	 // for print()
		"__del__"	 // destructor
	};

}