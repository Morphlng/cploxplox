#pragma once

#include <string>
#include <vector>
#include <memory>

namespace CXX {

    class Object;

    class Instance;

    class Interpreter;

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

        virtual Object call(Interpreter& interpreter, const std::vector<Object>& arguments) = 0;

        virtual int arity() = 0; // 参数个数

        virtual size_t required_params() = 0;  // 必须参数个数

        virtual std::shared_ptr<Callable> bindThis(std::shared_ptr<Instance> instance) = 0;

        virtual std::string to_string() = 0;

        virtual std::string name() = 0;

    public:
        CallableType type;
    };

}