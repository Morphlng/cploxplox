#pragma once
#include "Common/Error.h"

namespace CXX {

    class IllegalCharError : public Error
    {
    public:
        IllegalCharError(const Position& start, const Position& end, std::string details);
    };

}