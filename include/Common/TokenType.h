#pragma once

namespace CXX {

	enum class TokenType
	{
		// literals
		PLUS,
		MINUS,
		MUL,
		DIV,
		MOD,
		LPAREN,
		RPAREN,
		LBRACE,
		RBRACE,
		LBRACKET,
		RBRACKET,
		COMMA,
		DOT,
		COLON,
		SEMICOLON,
		PLUS_PLUS,
		MINUS_MINUS,
		PLUS_EQUAL,
		MINUS_EQUAL,
		MUL_EQUAL,
		DIV_EQUAL,
		QUESTION_MARK,

		// bool
		BANG,
		BANGEQ, // ! !=
		EQ,
		EQEQ, // = ==
		GT,
		GTE, // > >=
		LT,
		LTE, // < <=

		// primary
		NUMBER,
		STRING,
		IDENTIFIER,

		// keyword
		// constant
		NIL,
		TRUE,
		FALSE,
		// variable associate
		VAR,
		CLASS,
		THIS,
		SUPER,
		// control statement
		IF,
		ELSE,
		// loop
		FOR,
		WHILE,
		BREAK,
		CONTINUE,
		// function
		FUNC,
		RETURN,
		// logic
		AND,
		OR,
		// module
		IMPORT,
		AS,
		FROM,
		// stop sign
		END_OF_FILE
	};

	const char* TypeName(TokenType type);

}