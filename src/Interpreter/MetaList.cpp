#include "Interpreter/MetaList.h"
#include "Interpreter/Object.h"
#include "Interpreter/loxlib/NativeClass.h"
#include "Runner.h"
#include <algorithm>

namespace CXX {

	MetaList::MetaList(std::vector<Object> items) : Container("MetaList"), items(std::move(items)) {}

	void MetaList::reverse()
	{
		std::reverse(items.begin(), items.end());
	}

	size_t MetaList::length()
	{
		return items.size();
	}

	void MetaList::append(const Object& val)
	{
		items.push_back(val);
	}

	Object MetaList::pop()
	{
		if (items.empty()) {
			throw RuntimeError(Runner::pos_start, Runner::pos_end, "Poping from empty List");
		}

		Object ret = items.back();
		items.pop_back();
		return ret;
	}

	void MetaList::remove(const Object& val)
	{
		auto it = std::find(items.begin(), items.end(), val);
		if (it != items.end())
			items.erase(it);
	}

	void MetaList::unshift(const Object& val)
	{
		items.insert(items.begin(), val);
	}

	Object& MetaList::at(int index)
	{
		assertBound(index);
		return items.at(index);
	}

	Object MetaList::indexOf(const Object& val, int fromIndex)
	{
		assertBound(fromIndex);
		auto pos = std::find(items.begin() + fromIndex, items.end(), val);
		if (pos != items.end())
			return Object((double)(pos - items.begin()));

		return Object(-1.0);
	}

	Object MetaList::lastIndexOf(const Object& val, int fromIndex)
	{
		assertBound(fromIndex);
		auto pos = std::find(items.rbegin() + fromIndex, items.rend(), val);
		if (pos != items.rend())
			return Object((double)(items.size() - (pos - items.rbegin()) - 1));

		return Object(-1.0);
	}

	Object MetaList::reduce(std::shared_ptr<Callable> func)
	{
		size_t length = items.size();
		if (length == 0)
			return Object();
		else if (length == 1)
			return items[0];

		Object reduction = func->call(Runner::interpreter, { items[0], items[1] });
		for (size_t i = 2; i < length; i++)
		{
			reduction = func->call(Runner::interpreter, { reduction, items[i] });
		}

		return reduction;
	}

	Object MetaList::map(std::shared_ptr<Callable> func)
	{
		std::vector<Object> newitems;
		for (size_t i = 0; i < items.size(); i++)
		{
			newitems.push_back(func->call(Runner::interpreter, { items[i] }));
		}

		InstancePtr instance = List::instantiate(std::move(newitems));

		return Object(std::move(instance));
	}

	Object MetaList::slice(int fromIndex, int endIndex)
	{
		assertBound(fromIndex);
		assertBound(endIndex);
		if (fromIndex > endIndex) {
			throw RuntimeError(Runner::pos_start, Runner::pos_end, "invalid range of List");
		}

		return Object(List::instantiate(std::vector<Object>(items.begin() + fromIndex, items.begin() + endIndex)));
	}

	std::string MetaList::to_string()
	{
		std::string result = "[";
		for (size_t i = 0; i < items.size(); i++)
		{
			Object& item = items[i];

			// 要防止列表中包含自己导致的无限循环
			if (Classifier::belongClass(item, "List"))
			{
				auto list = item.getInstance()->get("@items");
				if (list.getContainer().get() == this)
					result += "...";
				else
					result += item.to_string();
			}
			else
				result += item.to_string();

			if (i != items.size() - 1)
				result += ", ";
		}
		result.push_back(']');

		return result;
	}

	void MetaList::assertBound(int& index)
	{
		// 如果是负下标，assertBound会将其改为正值
		if (index < 0)
			index = items.size() + index;

		if (index < 0 || index >= items.size())
		{
			throw RuntimeError(Runner::pos_start, Runner::pos_end, "List index out of bound");
		}
	}

	bool operator==(const MetaList& lhs, const MetaList& rhs)
	{
		if (lhs.items.size() != rhs.items.size())
			return false;

		for (size_t i = 0; i < lhs.items.size(); i++)
		{
			const Object& item = lhs.items[i];

			// 要防止列表中包含自己导致的无限循环
			if (Classifier::belongClass(item, "List"))
			{
				auto list = item.getInstance()->get("@items");
				if (list.getContainer().get() == &lhs)
					return false;
			}

			if (item != rhs.items[i])
				return false;
		}

		return true;
	}

	bool isMetaList(const Object& obj)
	{
		if (!obj.isContainer())
			return false;

		return obj.getContainer()->type == "MetaList";
	}

	MetaListPtr getMetaList(const Object& obj)
	{
		// 该函数仅在isMetaList判断后调用
		// 所以没有做类型判断
		return std::dynamic_pointer_cast<MetaList>(obj.getContainer());
	}

}