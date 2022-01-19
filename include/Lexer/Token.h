#pragma once

#include <string>
#include "Common/TokenType.h"
#include "Common/Position.h"

namespace CXX {

	class Token
	{
	public:
		Token(const TokenType& type = TokenType::NIL, std::string lexeme = "NIL",
			const Position& start = Position::preset, const Position& end = Position::preset);

		Token(const Token& other);

		[[nodiscard]] std::string to_string() const;

		bool operator==(const Token& rhs) const;

		bool operator<(const Token& rhs) const;

	public:
		TokenType type;
		std::string lexeme; // 词素(也就是value)
		Position pos_start;
		Position pos_end;
	};

}