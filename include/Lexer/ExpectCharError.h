#pragma once
#include "Common/Error.h"

namespace CXX {

	class ExpectCharError : public Error
	{
	public:
		ExpectCharError(const Position& start, const Position& end, std::string message);
	};

}