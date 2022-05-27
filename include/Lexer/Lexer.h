#pragma once

#include "Lexer/Token.h"
#include "Lexer/IllegalCharError.h"
#include "Lexer/ExpectCharError.h"
#include <vector>
#include <unordered_map>
#include <optional>

namespace CXX
{

	class Lexer
	{
	public:
		Lexer(const std::string &filename, const std::string &text);

		std::vector<Token> &tokenize();

		static std::unordered_map<std::string, TokenType> reservedKeywords;

	private:
		std::string_view text;
		std::string_view filename;
		Position pos;
		char current_char;
		std::vector<Token> tokens;

	private:
		void advance();

		void skip_comment();

		void make_string();

		void make_identifier();

		void make_number();

		void make_optional_token(char expect, TokenType optional, std::string optional_lexeme,
								 TokenType fallback, std::string fallback_lexeme);

		void make_plus_plus();

		void make_minus_minus();
	};

}