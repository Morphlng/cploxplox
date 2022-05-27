#pragma once

#include <vector>
#include <functional>
#include <algorithm>
#include "Common/typedefs.h"
#include "Lexer/Token.h"
#include "Parser/ParsingError.h"

namespace CXX
{

	class Parser
	{
	public:
		explicit Parser(const std::vector<Token> &tokens);
		explicit Parser(std::vector<Token> &&tokens);

		~Parser() = default;

		std::vector<StmtPtr> parse();

	private:
		StmtPtr declaration();

		StmtPtr varDeclStatement();

		StmtPtr funcDeclStatement();

		StmtPtr classDeclStatement();

		StmtPtr statement();

		StmtPtr exprStatement();

		StmtPtr ifStatement();

		StmtPtr whileStatement();

		StmtPtr forStatement();

		StmtPtr breakStatement();

		StmtPtr continueStatement();

		StmtPtr returnStatement();

		StmtPtr importStatement();

		std::vector<StmtPtr> block();

		ExprPtr expression();

		ExprPtr comma();

		ExprPtr assignment();

		ExprPtr ternary();

		ExprPtr logicOr();

		ExprPtr logicAnd();

		ExprPtr equality();

		ExprPtr comparison();

		ExprPtr term();

		ExprPtr factor();

		ExprPtr unary();

		ExprPtr prefix();

		ExprPtr postfix();

		ExprPtr call();

		std::vector<ExprPtr> arguments(TokenType ending);

		ExprPtr fillArgs(ExprPtr expr);

		ExprPtr primary();

		ExprPtr list_expr();

		ExprPtr bin_op(const std::function<ExprPtr(Parser *)> &funcA, std::initializer_list<TokenType> ops,
					   const std::function<ExprPtr(Parser *)> &funcB);

		ExprPtr func_body();

	private:
		Token current_tok;
		int tok_idx;
		std::vector<Token> tokens;

	private:
		void advance();

		void reverse(int step);

		Token previous();

		[[nodiscard]] bool check(const TokenType &type) const;

		void expect(const TokenType &type, const std::string &error_message);

		template <typename... Args>
		bool match(Args... args)
		{
			std::initializer_list<bool> results{check(args)...};
			if (std::any_of(std::begin(results), std::end(results), [&](bool type)
							{ return type; }))
			{
				advance();
				return true;
			}

			return false;
		}

		bool match(const std::initializer_list<TokenType> &types);

		void synchronize();
	};

}