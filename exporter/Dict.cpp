#include "Dict.h"

bool isMap(const Object &obj)
{
	if (!obj.isContainer())
		return false;

	return obj.getContainer()->type == "Map";
}

MapPtr getMap(const Object &obj)
{
	return std::dynamic_pointer_cast<Map>(obj.getContainer());
}

Dict::Dict() : NativeClass("Dict")
{
	allowedFields.emplace("@map", CXX::ObjectType::CONTAINER);

	methods.emplace("init", std::make_shared<NativeMethod>([&](Interpreter &interpreter, const std::vector<Object> &args)
														   {
															   Object &instance = interpreter.currContext()->getAt("this", 0);
															   instance.getInstance()->set("@map", Object(std::make_shared<Map>()));

															   return Object();
														   },
														   0));

	methods.emplace("length", std::make_shared<NativeMethod>([&](Interpreter &interpreter, const std::vector<Object> &args)
															 {
																 Object &instance = interpreter.currContext()->getAt("this", 0);
																 MapPtr m_map = getMap(instance.getInstance()->get("@map"));

																 return Object((double)(m_map->size()));
															 },
															 0));

	methods.emplace("set", std::make_shared<NativeMethod>([&](Interpreter &interpreter, const std::vector<Object> &args)
														  {
															  Object &instance = interpreter.currContext()->getAt("this", 0);
															  MapPtr m_map = getMap(instance.getInstance()->get("@map"));
															  m_map->set(args[0], args[1]);

															  return Object();
														  },
														  2));

	methods.emplace("get", std::make_shared<NativeMethod>([&](Interpreter &interpreter, const std::vector<Object> &args)
														  {
															  Object &instance = interpreter.currContext()->getAt("this", 0);
															  MapPtr m_map = getMap(instance.getInstance()->get("@map"));

															  return m_map->get(args[0]);
														  },
														  1));

	methods.emplace("__repr__", std::make_shared<NativeMethod>([&](Interpreter &interpreter, const std::vector<Object> &args)
															   {
																   Object &instance = interpreter.currContext()->getAt("this", 0);
																   MapPtr m_map = getMap(instance.getInstance()->get("@map"));

																   return Object(m_map->to_string());
															   },
															   0));
}

NativeClass *getClass_0()
{
	return new Dict;
}

const char *getClassName_0()
{
	return "Dict";
}