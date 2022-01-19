#include "Lexer/Token.h"
#include <utility>
#include "Common/utils.h"

namespace CXX {

	Token::Token(const TokenType& type, std::string lexeme, const Position& start, const Position& end) : type(type),
		lexeme(std::move(lexeme))
	{
		if (start.index != -1)
		{
			this->pos_start = start;
			this->pos_end = start;
			this->pos_end.advance();
		}

		if (end.index != -1)
			this->pos_end = end;
	}

	Token::Token(const Token& other)
	{
		this->type = other.type;
		this->lexeme = other.lexeme;
		this->pos_start = other.pos_start;
		this->pos_end = other.pos_end;
	}

	std::string Token::to_string() const
	{
		return format("Token: %s %s", TypeName(type), lexeme.c_str());
	}

	bool Token::operator==(const Token& rhs) const
	{
		return type == rhs.type && lexeme == rhs.lexeme;
	}

	bool Token::operator<(const Token& rhs) const
	{
		if (type != rhs.type)
			return type < rhs.type;
		if (lexeme != rhs.lexeme)
			return lexeme > rhs.lexeme;

		return pos_start.index < rhs.pos_start.index;
	}

}