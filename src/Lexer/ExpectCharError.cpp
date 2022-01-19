#include "Lexer/ExpectCharError.h"

namespace CXX {

	ExpectCharError::ExpectCharError(const Position& start, const Position& end, std::string details) :
		Error(start, end, "Expect Character", std::move(details))
	{}

}