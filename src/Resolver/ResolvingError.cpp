#include "Resolver/ResolvingError.h"

namespace CXX {

    ResolvingError::ResolvingError(const Position& start, const Position& end, std::string details) :
        Error(start, end, "Resolving Error", std::move(details))
    {}

}