#pragma once
#include "Common/Error.h"

namespace CXX {

	class RuntimeError : public Error
	{
	public:
		RuntimeError(const Position& start, const Position& end, std::string details);
		RuntimeError(Position* start, Position* end, std::string details);
	};

}