#include "Interpreter/RuntimeError.h"

namespace CXX {

	RuntimeError::RuntimeError(const Position& start, const Position& end, std::string details) :
		Error(start, end, "Runtime Error", std::move(details))
	{}

	RuntimeError::RuntimeError(Position* start, Position* end, std::string details) :
		Error(*start, *end, "Runtime Error", std::move(details))
	{}

}