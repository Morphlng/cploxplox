#include "Common/utils.h"
#include "Interpreter/loxlib/NativeClass.h"
#include "Interpreter/MetaList.h"
#include "Runner.h"

#include <cmath> // 部分函数要求c11
#include <random>
#include <algorithm>

namespace CXX {

	String::String() : NativeClass("String")
	{
		allowedFields.insert({ "str", ObjectType::STRING });

		methods.insert(
			{ "init", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
													{
														Object& instance = interpreter.context->get("this");
														instance.getInstance()->set("str", Object(args[0].to_string()));

														return Object();
													},
													1) });

		methods.insert(
			{ "length", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
													  {
														  Object& instance = interpreter.context->get("this");
														  // 因为初始化时已经转为字符串，所以这里一定拿到一个string
														  Object str = instance.getInstance()->get("str");

														  return Object((double)str.getString().length());
													  },
													  0) });

		methods.insert(
			{ "trim", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
													{
														Object& instance = interpreter.context->get("this");
														// 因为初始化时已经转为字符串，所以这里一定拿到一个string
														Object str = instance.getInstance()->get("str");

														// 示例如何返回一个实例对象
														// 请尽量保持返回值符合预期
														return Object(instantiate(strip(str.getString())));
													},
													0) });

		methods.insert(
			{ "split", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
													 {
														 if (!args[0].isString())
														 {
															 throw RuntimeError(Runner::pos_start, Runner::pos_end, "Expecting a string delim to split string");
														 }

														 Object& instance = interpreter.context->get("this");
														 // 因为初始化时已经转为字符串，所以这里一定拿到一个string
														 Object str = instance.getInstance()->get("str");

														 std::vector<std::string> split_result = split(str.getString(), args[0].getString());
														 std::vector<Object> retList;
														 for (auto& str : split_result)
														 {
															 retList.push_back(Object(str));
														 }

														 return Object(List::instantiate(std::move(retList)));
													 },
													 1) });

		// reservedMethods
		methods.insert(
			{ "__add__", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
													   {
														   Object& instance = interpreter.context->get("this");
														   InstancePtr instancePtr = instance.getInstance();
														   Object lhs = instancePtr->get("str");
														   auto& rhs = args[0];
														   Object prop; // 暂时存储string结果

														   if (args[0].isString())
														   {
															   prop = lhs + rhs;
														   }
														   else if (rhs.isInstance() && rhs.getInstance()->belonging == instancePtr->belonging)
														   {
															   prop = lhs + rhs.getInstance()->get("str");
														   }
														   else
														   {
															   throw RuntimeError(Runner::pos_start, Runner::pos_end,
																				  format("Illegal operator '+' for operands InstanceOf(%s) and type(%s)",
																						 className.c_str(),
																						 ObjectTypeName(rhs.type)));
														   }

														   return Object(instantiate(prop));
													   },
													   1) });

		methods.insert(
			{ "__mul__", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
													   {
														   Object& instance = interpreter.context->get("this");
														   InstancePtr instancePtr = instance.getInstance();
														   Object lhs = instancePtr->get("str");
														   auto& rhs = args[0];
														   Object prop; // 暂时存储string结果

														   if (args[0].isNumber())
														   {
															   prop = lhs * rhs;
														   }
														   else
														   {
															   throw RuntimeError(Runner::pos_start, Runner::pos_end,
																				  format("Illegal operator '+' for operands InstanceOf(%s) and type(%s)",
																						 className.c_str(),
																						 ObjectTypeName(rhs.type)));
														   }

														   return Object(instantiate(prop));
													   },
													   1) });

		methods.insert(
			{ "__equal__",
			 std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
											{
												Object& instance = interpreter.context->get("this");
												Object lhs = instance.getInstance()->get("str");

												// == 要求同类进行比较，因此rhs一定是instance
												// 但rhs不一定是lhs同类实例
												Object rhs = args[0].getInstance()->get("str");

												// equal返回的Object必须是ObjectType::BOOLEAN
												return Object(lhs == rhs);
											},
											1) });
	}

	std::shared_ptr<String> String::getSingleton()
	{
		static std::shared_ptr<String> singleton = std::make_shared<String>();
		return singleton;
	}

	InstancePtr String::instantiate(const std::string& str)
	{
		InstancePtr instance = std::make_shared<Instance>(String::getSingleton());
		instance->set("str", Object(str));

		return instance;
	}

	InstancePtr String::instantiate(const Object& obj)
	{
		// 应当要求obj.isString();
		InstancePtr instance = std::make_shared<Instance>(String::getSingleton());
		instance->set("str", obj);

		return instance;
	}

	List::List() : NativeClass("List")
	{
		// 不希望用户能取到MetaList，这里用一个用户无法使用的属性名
		allowedFields.insert({ "@items", ObjectType::CONTAINER });

		methods.insert(
			{ "init", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
													{
														Object& instanceObject = interpreter.context->get("this");
														InstancePtr instancePtr = instanceObject.getInstance();
														if (args.size() == 1 && isMetaList(args[0]))
														{
															instancePtr->set("@items", args[0]);
															return Object();
														}

														std::vector<Object> items;
														items.reserve(args.size());
														for (auto& arg : args)
														{
															items.push_back(arg);
														}
														MetaListPtr list = std::make_shared<MetaList>(std::move(items));
														instancePtr->set("@items", Object(list));

														return Object();
													},
													-1) });

		methods.insert(
			{ "length", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
													  {
														  Object& instance = interpreter.context->get("this");
														  // 因为初始化时已经转为列表，所以这里一定拿到一个MetaList
														  MetaListPtr list = getMetaList(instance.getInstance()->get("@items"));

														  return Object((double)list->length());
													  },
													  0) });

		methods.insert(
			{ "reverse", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
													   {
														   Object& instance = interpreter.context->get("this");
														   // 因为初始化时已经转为列表，所以这里一定拿到一个MetaList
														   MetaListPtr list = getMetaList(instance.getInstance()->get("@items"));
														   list->reverse();

														   return Object();
													   },
													   0) });

		methods.insert(
			{ "append", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
													  {
														  Object& instance = interpreter.context->get("this");
														  // 因为初始化时已经转为列表，所以这里一定拿到一个MetaList
														  MetaListPtr list = getMetaList(instance.getInstance()->get("@items"));
														  list->append(args[0]);

														  return Object();
													  },
													  1) });

		methods.insert(
			{ "remove", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
													  {
														  Object& instance = interpreter.context->get("this");
														  // 因为初始化时已经转为列表，所以这里一定拿到一个MetaList
														  MetaListPtr list = getMetaList(instance.getInstance()->get("@items"));
														  list->remove(args[0]);
														  return Object();
													  },
													  1) });

		methods.insert(
			{ "pop", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
													  {
														  Object& instance = interpreter.context->get("this");
														  // 因为初始化时已经转为列表，所以这里一定拿到一个MetaList
														  MetaListPtr list = getMetaList(instance.getInstance()->get("@items"));
														  return list->pop();
													  },
													  0) });

		methods.insert(
			{ "unshift", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
													   {
														   Object& instance = interpreter.context->get("this");
														   // 因为初始化时已经转为列表，所以这里一定拿到一个MetaList
														   MetaListPtr list = getMetaList(instance.getInstance()->get("@items"));
														   list->unshift(args[0]);

														   return Object();
													   },
													   1) });

		methods.insert(
			{ "indexOf", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
													   {
														   Object& instance = interpreter.context->get("this");
														   // 因为初始化时已经转为列表，所以这里一定拿到一个MetaList
														   MetaListPtr list = getMetaList(instance.getInstance()->get("@items"));

														   if (args.size() == 2)
														   {
															   if (!args[1].isNumber())
															   {
																   throw RuntimeError(Runner::pos_start, Runner::pos_end, "argument fromIndex must be a number");
															   }

															   return list->indexOf(args[0], args[1].getNumber());
														   }
														   else
															   return list->indexOf(args[0]);
													   },
													   2, 1) });

		methods.insert(
			{ "lastIndexOf", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
														   {
															   Object& instance = interpreter.context->get("this");
															   // 因为初始化时已经转为列表，所以这里一定拿到一个MetaList
															   MetaListPtr list = getMetaList(instance.getInstance()->get("@items"));

															   if (args.size() == 2)
															   {
																   if (!args[1].isNumber())
																   {
																	   throw RuntimeError(Runner::pos_start, Runner::pos_end, "argument fromIndex must be a number");
																   }

																   return list->lastIndexOf(args[0], args[1].getNumber());
															   }
															   else
																   return list->lastIndexOf(args[0]);
														   },
														   2, 1) });

		methods.insert(
			{ "reduce", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
													  {
														  if (!args[0].isCallable())
														  {
															  throw RuntimeError(Runner::pos_start, Runner::pos_end, "Expecting a function to reduce");
														  }
														  CallablePtr func = args[0].getCallable();
														  if (func->type != CallableType::FUNCTION || func->arity() != 2)
														  {
															  throw RuntimeError(Runner::pos_start, Runner::pos_end, "Expecting a function with two parameters to reduce");
														  }

														  Object& instance = interpreter.context->get("this");
														  // 因为初始化时已经转为列表，所以这里一定拿到一个MetaList
														  MetaListPtr list = getMetaList(instance.getInstance()->get("@items"));

														  return list->reduce(std::move(func));
													  },
													  1) });

		methods.insert(
			{ "map", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
												   {
													   if (!args[0].isCallable())
													   {
														   throw RuntimeError(Runner::pos_start, Runner::pos_end, "Expecting a function to map");
													   }

													   CallablePtr func = args[0].getCallable();
													   if (func->type != CallableType::FUNCTION || func->arity() != 1)
													   {
														   throw RuntimeError(Runner::pos_start, Runner::pos_end, "Expecting a function with one parameters to map");
													   }

													   Object& instance = interpreter.context->get("this");
													   // 因为初始化时已经转为列表，所以这里一定拿到一个MetaList
													   MetaListPtr list = getMetaList(instance.getInstance()->get("@items"));

													   return list->map(std::move(func));
												   },
												   1) });

		methods.insert(
			{ "slice", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
												   {
													   if (!args[0].isNumber() || !args[1].isNumber())
													   {
														   throw RuntimeError(Runner::pos_start, Runner::pos_end, "range should be represented using Nubmer");
													   }

													   Object& instance = interpreter.context->get("this");
													   // 因为初始化时已经转为列表，所以这里一定拿到一个MetaList
													   MetaListPtr list = getMetaList(instance.getInstance()->get("@items"));

													   return list->slice(args[0].getNumber(),args[1].getNumber());
												   },
												   2) });

		// reservedMethods
		methods.insert(
			{ "__equal__",
			 std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
											{
												if (!Classifier::belongClass(args[0], "List"))
													return Object(false);

												Object& instance = interpreter.context->get("this");
												// get two MetaListPtr
												MetaListPtr lhs = getMetaList(instance.getInstance()->get("@items"));
												MetaListPtr rhs = getMetaList(args[0].getInstance()->get("@items"));

												// 指向同一个MetaList显然相同
												if (lhs == rhs)
													return Object(true);

												return Object(*lhs == *rhs);
											},
											1) });

		methods.insert(
			{ "__repr__", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
														{
															Object& instance = interpreter.context->get("this");
															// 因为初始化时已经转为列表，所以这里一定拿到一个MetaList
															Object list = instance.getInstance()->get("@items");

															return Object(list.to_string());
														},
														0) });
	}

	std::shared_ptr<List> List::getSingleton()
	{
		static std::shared_ptr<List> singleton = std::make_shared<List>();
		return singleton;
	}

	InstancePtr List::instantiate(std::vector<Object> items)
	{
		InstancePtr instance = std::make_shared<Instance>(List::getSingleton());

		instance->set("@items", Object(std::make_shared<MetaList>(std::move(items))));

		return instance;
	}

	Mathematics::Mathematics() : NativeClass("Mathematics")
	{
		// There is no allow field
		// Actually, Mathematics can't be instanced by user

		methods.insert(
			{ "abs", std::make_shared<NativeMethod>([](Interpreter& interpreter, const std::vector<Object>& args)
												   {
													   if (!args[0].isNumber())
													   {
														   return Object();
													   }
													   double val = args[0].getNumber();

													   return val < 0 ? Object(-val) : args[0];
												   },
												   1) });

		// 四舍五入
		methods.insert(
			{ "round", std::make_shared<NativeMethod>([](Interpreter& interpreter, const std::vector<Object>& args)
													 {
														 if (!args[0].isNumber())
														 {
															 return Object();
														 }
														 double val = args[0].getNumber();

														 return Object(round(val));
													 },
													 1) });

		// 小于等于x的最大整数
		methods.insert(
			{ "floor", std::make_shared<NativeMethod>([](Interpreter& interpreter, const std::vector<Object>& args)
													 {
														 if (!args[0].isNumber())
														 {
															 return Object();
														 }
														 double val = args[0].getNumber();

														 return Object(floor(val));
													 },
													 1) });

		// 大于等于x的最小整数
		methods.insert(
			{ "ceil", std::make_shared<NativeMethod>([](Interpreter& interpreter, const std::vector<Object>& args)
													{
														if (!args[0].isNumber())
														{
															return Object();
														}
														double val = args[0].getNumber();

														return Object(ceil(val));
													},
													1) });

		methods.insert(
			{ "pow", std::make_shared<NativeMethod>([](Interpreter& interpreter, const std::vector<Object>& args)
												   {
													   if (!args[0].isNumber() || !args[1].isNumber())
													   {
														   return Object();
													   }
													   double base = args[0].getNumber();
													   double power = args[1].getNumber();

													   return Object(pow(base, power));
												   },
												   2) });

		methods.insert(
			{ "sqrt", std::make_shared<NativeMethod>([](Interpreter& interpreter, const std::vector<Object>& args)
													{
														if (!args[0].isNumber())
														{
															return Object();
														}
														double val = args[0].getNumber();
														if (val < 0)
															return Object();

														return Object(sqrt(val));
													},
													1) });

		methods.insert(
			{ "exp", std::make_shared<NativeMethod>([](Interpreter& interpreter, const std::vector<Object>& args)
												   {
													   if (!args[0].isNumber())
													   {
														   return Object();
													   }
													   double val = args[0].getNumber();

													   return Object(exp(val));
												   },
												   1) });

		methods.insert(
			{ "sin", std::make_shared<NativeMethod>([](Interpreter& interpreter, const std::vector<Object>& args)
												   {
													   if (!args[0].isNumber())
													   {
														   return Object();
													   }
													   double val = args[0].getNumber();

													   return Object(sin(val));
												   },
												   1) });

		methods.insert(
			{ "cos", std::make_shared<NativeMethod>([](Interpreter& interpreter, const std::vector<Object>& args)
												   {
													   if (!args[0].isNumber())
													   {
														   return Object();
													   }
													   double val = args[0].getNumber();

													   return Object(cos(val));
												   },
												   1) });

		methods.insert(
			{ "tan", std::make_shared<NativeMethod>([](Interpreter& interpreter, const std::vector<Object>& args)
												   {
													   if (!args[0].isNumber())
													   {
														   return Object();
													   }
													   double val = args[0].getNumber();

													   return Object(tan(val));
												   },
												   1) });

		// 求以e为底的对数
		methods.insert(
			{ "log", std::make_shared<NativeMethod>([](Interpreter& interpreter, const std::vector<Object>& args)
												   {
													   if (!args[0].isNumber())
													   {
														   return Object();
													   }
													   double val = args[0].getNumber();

													   return Object(log(val));
												   },
												   1) });

		// 求以2为底的对数
		methods.insert(
			{ "log2", std::make_shared<NativeMethod>([](Interpreter& interpreter, const std::vector<Object>& args)
													{
														if (!args[0].isNumber())
														{
															return Object();
														}
														double val = args[0].getNumber();

														return Object(log2(val));
													},
													1) });

		methods.insert(
			{ "min", std::make_shared<NativeMethod>([](Interpreter& interpreter, const std::vector<Object>& args)
												   {
													   bool notAllNum = std::any_of(args.begin(), args.end(), [](const Object& arg)
																					{ return !arg.isNumber(); });

													   // 从非数字类型元素中获取最小/大没有意义
													   if (notAllNum)
													   {
														   return Object();
													   }

													   // 如果直接比较Object，会造成反复获取值的问题
													   std::vector<double> numbers;
													   for (auto& arg : args)
													   {
														   numbers.push_back(arg.getNumber());
													   }

													   return Object(*std::min_element(numbers.begin(), numbers.end()));
												   },
												   -1) });

		methods.insert(
			{ "max", std::make_shared<NativeMethod>([](Interpreter& interpreter, const std::vector<Object>& args)
												   {
													   bool notAllNum = std::any_of(args.begin(), args.end(), [](const Object& arg)
																					{ return !arg.isNumber(); });

													   // 从非数字类型元素中获取最小/大没有意义
													   if (notAllNum)
													   {
														   return Object();
													   }

													   // 如果直接比较Object，会造成反复获取值的问题
													   std::vector<double> numbers;
													   for (auto& arg : args)
													   {
														   numbers.push_back(arg.getNumber());
													   }

													   return Object(*std::max_element(numbers.begin(), numbers.end()));
												   },
												   -1) });

		// Init random device only once
		std::random_device rd;									 // 真随机硬件
		static std::mt19937_64 gen(rd());						 // 伪随机生成器(梅森旋转)
		static std::uniform_real_distribution<double> dis(0, 1); // 随机数分布于[0,1)

		methods.insert(
			{ "random", std::make_shared<NativeMethod>([&](Interpreter& interpreter, const std::vector<Object>& args)
													  { return Object(dis(gen)); },
													  0) });
	}

	InstancePtr Mathematics::instantiate()
	{
		// No shared_from_this, since this should be the only instance created
		InstancePtr Math = std::make_shared<Instance>(std::make_shared<Mathematics>());
		Math->fields["PI"] = Object(3.141592653589793);		 // pi
		Math->fields["E"] = Object(2.718281828459045);		 // e
		Math->fields["LN2"] = Object(0.6931471805599453);	 // 以e为底，2的对数
		Math->fields["LN10"] = Object(2.302585092994046);	 // 以e为底，10的对数
		Math->fields["LOG2E"] = Object(1.4426950408889634);	 // 以2为底，e的对数
		Math->fields["LOG10E"] = Object(0.4342944819032518); // 以10为底，e的对数

		return Math;
	}

	std::string Classifier::className(const Object& val)
	{
		if (val.isInstance())
		{
			return val.getInstance()->belonging->className;
		}

		return std::string();
	}

	bool Classifier::belongClass(const Object& obj, const char* expect)
	{
		return className(obj) == expect;
	}

}