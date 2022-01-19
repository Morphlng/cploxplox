#pragma once
#include "Common/Error.h"

namespace CXX {

    class ParsingError : public Error
    {
    public:
        ParsingError(const Position& start, const Position& end, std::string details);
    };

}