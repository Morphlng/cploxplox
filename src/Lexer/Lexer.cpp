#include "Lexer/Lexer.h"
#include "Common/utils.h"
#include <cctype> // for isalpha() & isdigit()

namespace CXX
{

	Lexer::Lexer(const std::string &filename, const std::string &text) : text(text), filename(filename), pos(-1, 0, -1, filename, text), current_char('\0')
	{
		advance();
	}

	void Lexer::advance()
	{
		this->pos.advance(this->current_char);
		if (this->pos.index < this->text.length())
			this->current_char = this->text[this->pos.index];
		else
			this->current_char = '\0';
	}

	std::vector<Token> &Lexer::tokenize()
	{
		while (current_char != '\0')
		{
			switch (current_char)
			{
			case ' ':
			case '\r':
			case '\t':
			case '\n':
				advance();
				break; // ignores
			case '#':
				skip_comment();
				break;

				// literals
			case '(':
				tokens.emplace_back(TokenType::LPAREN, "(", pos);
				advance();
				break;
			case ')':
				tokens.emplace_back(TokenType::RPAREN, ")", pos);
				advance();
				break;
			case '{':
				tokens.emplace_back(TokenType::LBRACE, "{", pos);
				advance();
				break;
			case '}':
				tokens.emplace_back(TokenType::RBRACE, "}", pos);
				advance();
				break;
			case '[':
				tokens.emplace_back(TokenType::LBRACKET, "[", pos);
				advance();
				break;
			case ']':
				tokens.emplace_back(TokenType::RBRACKET, "]", pos);
				advance();
				break;
			case ',':
				tokens.emplace_back(TokenType::COMMA, ",", pos);
				advance();
				break;
			case '.':
				tokens.emplace_back(TokenType::DOT, ".", pos);
				advance();
				break;
			case ';':
				tokens.emplace_back(TokenType::SEMICOLON, ";", pos);
				advance();
				break;
			case ':':
				tokens.emplace_back(TokenType::COLON, ":", pos);
				advance();
				break;
			case '+':
				make_plus_plus();
				break;
			case '-':
				make_minus_minus();
				break;
			case '*':
				make_optional_token('=', TokenType::MUL_EQUAL, "*=", TokenType::MUL, "*");
				break;
			case '/':
				make_optional_token('=', TokenType::DIV_EQUAL, "/=", TokenType::DIV, "/");
				break;
			case '%':
				tokens.emplace_back(TokenType::MOD, "%", pos);
				advance();
				break;
			case '?':
				tokens.emplace_back(TokenType::QUESTION_MARK, "?", pos);
				advance();
				break;

				// one or two character (e.g. <=)
			case '!':
				make_optional_token('=', TokenType::BANGEQ, "!=", TokenType::BANG, "!");
				break;

			case '=':
				make_optional_token('=', TokenType::EQEQ, "==", TokenType::EQ, "=");
				break;

			case '<':
				make_optional_token('=', TokenType::LTE, "<!>=", TokenType::LT, "<");
				break;

			case '>':
				make_optional_token('=', TokenType::GTE, ">=", TokenType::GT, ">");
				break;

			case '"':
				make_string();
				break;

			default:
				if (isdigit((unsigned char)current_char))
				{
					make_number();
				}
				else if (isalpha((unsigned char)current_char) || current_char == '_')
				{
					make_identifier();
				}
				else
				{
					Position start(this->pos);
					advance();
					// 这里不在错误中传递非法字符了
					// 因此snprintf这个函数遇到中午字符会有bug
					throw IllegalCharError(start, this->pos, "Cannot tokenize this character");
				}
			}
		}

		tokens.emplace_back(TokenType::END_OF_FILE, "", pos);

		return tokens;
	}

	void Lexer::skip_comment()
	{
		advance(); // 跳过#

		while (current_char != '\0' && current_char != '\n')
		{
			advance();
		}

		advance(); // 跳过换行
	}

	void Lexer::make_string()
	{
		std::string str;
		Position start = pos;
		advance(); // 跳过'"'

		// 处理形如：str = "abc:\"xxx\""
		// 读取到\后的下一个字符需要无条件读入
		// 我们需要处理，如果\后面是n，则应为\n（换行）等情况

		bool escape = false; // 当遇到'\\'，设为true，意为接收下一个字符

		// 上一个字符是\\，则下一个n应被转换为\n；以此类推
		std::unordered_map<char, char> escape_characters{
			{'n', '\n'},
			{'t', '\t'}};

		while (current_char != '\0' && (current_char != '\"' || escape))
		{
			if (escape)
			{
				if (escape_characters.find(current_char) != escape_characters.end())
					str.push_back(escape_characters[current_char]);
				else
					str.push_back(current_char);
				escape = false;
			}
			else
			{
				if (current_char == '\\')
					escape = true;
				else
					str.push_back(current_char);
			}
			advance();
		}

		if (current_char != '\"')
			throw ExpectCharError(start, this->pos, "'\"' at the end of a string");

		advance(); // 跳过'"'

		tokens.emplace_back(TokenType::STRING, str, start, pos);
	}

	bool validForIdentifier(char c)
	{
		return isalpha(c) || isdigit(c) || c == '_';
	}

	void Lexer::make_identifier()
	{
		Position start = pos;
		std::string value;

		while (current_char != '\0' && validForIdentifier(current_char))
		{
			value.push_back(current_char);
			advance();
		}

		if (reservedKeywords.find(value) != reservedKeywords.end())
			tokens.emplace_back(reservedKeywords[value], value, start, pos);
		else
			tokens.emplace_back(TokenType::IDENTIFIER, value, start, pos);
	}

	void Lexer::make_number()
	{
		Position start = pos;

		while (current_char != '\0' && isdigit(current_char))
		{
			advance();
		}

		if (current_char == '.')
		{
			advance();
			while (current_char != '\0' && isdigit(current_char))
			{
				advance();
			}
		}
		else if (current_char == 'x') // 16进制
		{
			advance();
			while (current_char != '\0' && isxdigit(current_char))
			{
				advance();
			}
		}
		else if (current_char == 'b') // 2进制
		{
			advance();
			while (current_char != '\0' && (current_char == '0' || current_char == '1'))
			{
				advance();
			}
		}

		std::string_view number = text.substr(start.index, (pos.index - start.index));

		tokens.emplace_back(TokenType::NUMBER, std::string(number), start, pos);
	}

	void Lexer::make_optional_token(char expect, TokenType optional, std::string optional_lexeme, TokenType fallback,
									std::string fallback_lexeme)
	{
		Position start = pos;
		advance();

		if (current_char == expect)
		{
			advance();
			tokens.emplace_back(optional, optional_lexeme, start, pos);
		}
		else
		{
			tokens.emplace_back(fallback, fallback_lexeme, start);
		}
	}

	void Lexer::make_plus_plus()
	{
		Position start = pos;
		advance(); // 跳过第一个'+'

		if (current_char == '+')
		{
			advance();
			tokens.emplace_back(TokenType::PLUS_PLUS, "++", start, pos);
		}
		else if (current_char == '=')
		{
			advance();
			tokens.emplace_back(TokenType::PLUS_EQUAL, "+=", start, pos);
		}
		else
		{
			tokens.emplace_back(TokenType::PLUS, "+", start);
		}
	}

	void Lexer::make_minus_minus()
	{
		Position start = pos;
		advance(); // 跳过第一个'-'

		if (current_char == '-')
		{
			advance();
			tokens.emplace_back(TokenType::MINUS_MINUS, "--", start, pos);
		}
		else if (current_char == '=')
		{
			advance();
			tokens.emplace_back(TokenType::MINUS_EQUAL, "-=", start, pos);
		}
		else
		{
			tokens.emplace_back(TokenType::MINUS, "-", start);
		}
	}

	std::unordered_map<std::string, TokenType> Lexer::reservedKeywords = {
		{"nil", TokenType::NIL},
		{"true", TokenType::TRUE},
		{"false", TokenType::FALSE},
		{"var", TokenType::VAR},
		{"class", TokenType::CLASS},
		{"this", TokenType::THIS},
		{"super", TokenType::SUPER},
		{"if", TokenType::IF},
		{"else", TokenType::ELSE},
		{"for", TokenType::FOR},
		{"while", TokenType::WHILE},
		{"break", TokenType::BREAK},
		{"continue", TokenType::CONTINUE},
		{"func", TokenType::FUNC},
		{"return", TokenType::RETURN},
		{"and", TokenType::AND},
		{"or", TokenType::OR},
		{"import", TokenType::IMPORT},
		{"as", TokenType::AS},
		{"from", TokenType::FROM}};

}