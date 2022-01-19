#include "Parser/ParsingError.h"

namespace CXX {

	ParsingError::ParsingError(const Position& start, const Position& end, std::string details) :
		Error(start, end, "Parsing Error", std::move(details))
	{}

}