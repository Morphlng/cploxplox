#include "Common/TokenType.h"

namespace CXX {

	const char* TypeName(TokenType type)
	{
		switch (type)
		{
		case TokenType::PLUS: // +
			return "PLUS";
		case TokenType::MINUS: // -
			return "MINUS";
		case TokenType::MUL: // *
			return "MUL";
		case TokenType::DIV: // /
			return "DIV";
		case TokenType::MOD: // %
			return "MOD";
		case TokenType::LPAREN: // (
			return "LPAREN";
		case TokenType::RPAREN: // )
			return "RPAREN";
		case TokenType::LBRACE: // [
			return "LBRACE";
		case TokenType::RBRACE: // ]
			return "RBRACE";
		case TokenType::LBRACKET: // {
			return "LBRACKET";
		case TokenType::RBRACKET: // }
			return "RBRACKET";
		case TokenType::COMMA: // ,
			return "COMMA";
		case TokenType::DOT: // .
			return "DOT";
		case TokenType::COLON: //:
			return "PLUS";
		case TokenType::SEMICOLON: // ;
			return "SEMICOLON";
		case TokenType::PLUS_PLUS:
			return "PLUS_PLUS";
		case TokenType::MINUS_MINUS:
			return "MINUS_MINUS";
		case TokenType::PLUS_EQUAL:
			return "PLUS_EQUAL";
		case TokenType::MINUS_EQUAL:
			return "MINUS_EQUAL";
		case TokenType::MUL_EQUAL:
			return "MUL_EQUAL";
		case TokenType::DIV_EQUAL:
			return "DIV_EQUAL";
		case TokenType::QUESTION_MARK: // ?
			return "QUESTION_MARK";

			// bool
		case TokenType::BANG: // !
			return "BANG";
		case TokenType::BANGEQ: // !=
			return "BANGEQ";
		case TokenType::EQ: // =
			return "EQ";
		case TokenType::EQEQ: // ==
			return "EQEQ";
		case TokenType::GT: // >
			return "GT";
		case TokenType::GTE: // >=
			return "GTE";
		case TokenType::LT: // LT
			return "LT";
		case TokenType::LTE: // <=
			return "LTE";

			// primary
		case TokenType::NUMBER: // number
			return "NUMBER";
		case TokenType::STRING: // string
			return "STRING";
		case TokenType::IDENTIFIER: // identifier
			return "IDENTIFIER";

			// keyword
		case TokenType::NIL: // nil, represent undefined variable
			return "NIL";
		case TokenType::TRUE: // true
			return "TRUE";
		case TokenType::FALSE: // false
			return "FALSE";
		case TokenType::VAR:
			return "VAR";
		case TokenType::CLASS:
			return "CLASS";
		case TokenType::THIS:
			return "THIS";
		case TokenType::SUPER:
			return "IDENTIFIER";
		case TokenType::IF:
			return "IF";
		case TokenType::ELSE:
			return "ELSE";
		case TokenType::FOR:
			return "FOR";
		case TokenType::WHILE:
			return "WHILE";
		case TokenType::BREAK:
			return "BREAK";
		case TokenType::CONTINUE:
			return "CONTINUE";
		case TokenType::FUNC:
			return "FUNC";
		case TokenType::RETURN:
			return "RETURN";
		case TokenType::AND:
			return "AND";
		case TokenType::OR:
			return "OR";

		case TokenType::IMPORT:
			return "IMPORT";

		case TokenType::AS:
			return "AS";

		case TokenType::FROM:
			return "FROM";

		case TokenType::END_OF_FILE:
			return "EOF";
		}

		return "unreachable";
	}

}