#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Interpreter/Container.h"

namespace CXX {

    class Object;
    class Callable;

    // 实际处理时使用内部类List(instance)
    class MetaList :public Container
    {
        friend bool operator==(const MetaList& lhs, const MetaList& rhs);
    public:
        MetaList(std::vector<Object> items);
        ~MetaList() = default;

        // get/set
        void append(const Object& val);
        Object pop();
        void remove(const Object& val);
        void unshift(const Object& val);
        Object& at(int index);

        // util
        void reverse();
        Object indexOf(const Object& val, int fromIndex = 0);
        Object lastIndexOf(const Object& val, int fromIndex = 0);
        Object reduce(std::shared_ptr<Callable> func);
        Object map(std::shared_ptr<Callable> func);
        Object slice(int fromIndex, int endIndex);

        // properties
        size_t length();
        std::string to_string();

    private:
        std::vector<Object> items;

    private:
        void assertBound(int& index);
    };

    using MetaListPtr = std::shared_ptr<MetaList>;

    bool isMetaList(const Object& obj);

    MetaListPtr getMetaList(const Object& obj);

}