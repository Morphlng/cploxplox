#pragma once
#include "Common/Error.h"

namespace CXX {

    class ResolvingError : public Error
    {
    public:
        ResolvingError(const Position& start, const Position& end, std::string message);
    };

}