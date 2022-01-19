#include "Lexer/IllegalCharError.h"

namespace CXX {

	IllegalCharError::IllegalCharError(const Position& start, const Position& end, std::string details) :
		Error(start, end, "Illegal Character", std::move(details))
	{}

}