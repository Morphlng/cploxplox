#include <algorithm>
#include "Parser/Parser.h"
#include "Parser/ParsingError.h"
#include "Parser/Stmt.h"
#include "Parser/Expr.h"

namespace CXX
{

	Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens), tok_idx(-1)
	{
		advance();
	}

	Parser::Parser(std::vector<Token> &&tokens) : tokens(std::move(tokens)), tok_idx(-1)
	{
		advance();
	}

	std::vector<StmtPtr> Parser::parse()
	{
		std::vector<StmtPtr> statements;
		while (current_tok.type != TokenType::END_OF_FILE)
		{
			statements.push_back(declaration());
		}

		return statements; // 将亡值不要显示使用std::move
	}

	StmtPtr Parser::declaration()
	{
		try
		{
			switch (current_tok.type)
			{
			case TokenType::VAR:
			{
				advance();
				return varDeclStatement();
			}

			case TokenType::CLASS:
			{
				advance();
				return classDeclStatement();
			}

			case TokenType::FUNC:
			{
				advance();
				if (current_tok.type == TokenType::IDENTIFIER)
				{
					advance();
					return funcDeclStatement();
				}
				reverse(1); // go match LambdaFunction
				return statement();
			}

			default:
				return statement();
			}
		}
		catch (const ParsingError &e)
		{
			ErrorReporter::report(e);
			synchronize();

			return std::make_shared<ErrorStmt>(e.pos_start, e.pos_end);
		}
	}

	StmtPtr Parser::varDeclStatement()
	{
		std::vector<StmtPtr> statements;

		do
		{
			expect(TokenType::IDENTIFIER, "Expected identifier");
			Token identifier = previous();

			std::optional<ExprPtr> initializer;
			if (match(TokenType::EQ))
			{
				initializer = ternary();
			}
			statements.push_back(std::make_shared<VarDeclarationStmt>(identifier, std::move(initializer)));
		} while (match(TokenType::COMMA));

		expect(TokenType::SEMICOLON, "Expect ';' after variable declaration");

		return statements.size() == 1 ? std::move(statements[0]) : std::make_shared<PackStmt>(std::move(statements));
	}

	StmtPtr Parser::funcDeclStatement()
	{
		Token name = previous();

		std::shared_ptr<LambdaExpr> ptr = std::static_pointer_cast<LambdaExpr>(func_body());

		return std::make_shared<FuncDeclarationStmt>(name, std::move(ptr->params), std::move(ptr->default_values), std::move(ptr->body));
	}

	StmtPtr Parser::classDeclStatement()
	{
		expect(TokenType::IDENTIFIER, "Expect Class name");
		Token name = previous();

		std::optional<std::shared_ptr<VariableExpr>> superclass;
		if (match(TokenType::GT))
		{
			expect(TokenType::IDENTIFIER, "Expect SuperClass name");
			superclass = std::make_shared<VariableExpr>(previous());
		}

		expect(TokenType::LBRACE, "Expect '{' before class body");
		std::vector<std::shared_ptr<FuncDeclarationStmt>> methods;
		while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE))
		{
			expect(TokenType::IDENTIFIER, "Expect method name");
			methods.push_back(std::static_pointer_cast<FuncDeclarationStmt>(funcDeclStatement()));
		}

		expect(TokenType::RBRACE, "Expect '}' to close up class body");

		return std::make_shared<ClassDeclarationStmt>(name, std::move(methods), std::move(superclass));
	}

	StmtPtr Parser::statement()
	{
		switch (current_tok.type)
		{
		case TokenType::LBRACE:
			advance();
			return std::make_shared<BlockStmt>(block());

		case TokenType::IF:
			advance();
			return ifStatement();

		case TokenType::WHILE:
			advance();
			return whileStatement();

		case TokenType::FOR:
			advance();
			return forStatement();

		case TokenType::BREAK:
			advance();
			return breakStatement();

		case TokenType::CONTINUE:
			advance();
			return continueStatement();

		case TokenType::RETURN:
			advance();
			return returnStatement();

		case TokenType::IMPORT:
			advance();
			return importStatement();

		default:
			return exprStatement();
		}
	}

	StmtPtr Parser::exprStatement()
	{
		ExprPtr expr = expression();

		expect(TokenType::SEMICOLON, "Expect ';' at the end of an expression.");

		return std::make_shared<ExpressionStmt>(std::move(expr));
	}

	StmtPtr Parser::ifStatement()
	{
		expect(TokenType::LPAREN, "Expect '(' after if");
		ExprPtr condition = expression();
		expect(TokenType::RPAREN, "Expect ')' to close up condition");
		StmtPtr thenBranch = statement();

		std::optional<StmtPtr> elseBranch;
		if (match(TokenType::ELSE))
		{
			elseBranch = statement();
		}

		return std::make_shared<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
	}

	StmtPtr Parser::whileStatement()
	{
		expect(TokenType::LPAREN, "Expect '(' after while");
		ExprPtr condition = expression();
		expect(TokenType::RPAREN, "Expect ')' to close up condition");
		StmtPtr body = statement();

		return std::make_shared<WhileStmt>(std::move(condition), std::move(body));
	}

	StmtPtr Parser::forStatement()
	{
		expect(TokenType::LPAREN, "Expect '(' after for");
		std::optional<StmtPtr> initializer;
		if (match(TokenType::SEMICOLON))
		{
		}
		else if (match(TokenType::VAR))
		{
			initializer = varDeclStatement();
		}
		else
		{
			initializer = exprStatement();
		}

		std::optional<ExprPtr> condition;
		if (!check(TokenType::SEMICOLON))
		{
			condition = expression();
		}
		expect(TokenType::SEMICOLON, "Expect ';' after condition");

		std::optional<ExprPtr> increment;
		if (!check(TokenType::RPAREN))
		{
			increment = expression();
		}
		expect(TokenType::RPAREN, "Expect ')' after for clauses");

		StmtPtr body = statement();

		return std::make_shared<ForStmt>(std::move(initializer), std::move(condition), std::move(increment),
										 std::move(body));
	}

	StmtPtr Parser::breakStatement()
	{
		Token keyword = previous();
		expect(TokenType::SEMICOLON, "Expect ';' after break");
		return std::make_shared<BreakStmt>(keyword);
	}

	StmtPtr Parser::continueStatement()
	{
		Token keyword = previous();
		expect(TokenType::SEMICOLON, "Expect ';' after continue");
		return std::make_shared<ContinueStmt>(keyword);
	}

	StmtPtr Parser::returnStatement()
	{
		Token keyword = previous();
		std::optional<ExprPtr> expr;
		if (!check(TokenType::SEMICOLON))
		{
			expr = expression();
		}
		expect(TokenType::SEMICOLON, "Expected ';' after return statement");
		return std::make_shared<ReturnStmt>(keyword, std::move(expr));
	}

	StmtPtr Parser::importStatement()
	{
		Token keyword = previous(); // "import"
		expect(TokenType::LBRACE, "Expect '{' after import.");

		std::map<Token, std::optional<Token>> symbols;
		// import everything
		if (match(TokenType::MUL))
		{
			symbols.emplace(previous(), std::nullopt);
		}
		else
		{
			do
			{
				expect(TokenType::IDENTIFIER, "Expect symbol list");

				Token symbol = previous();
				std::optional<Token> alias; // 别名
				if (match(TokenType::AS))
				{
					expect(TokenType::IDENTIFIER, "Expect identifier for alias");
					alias = previous();
				}

				symbols.emplace(symbol, alias);
			} while (match(TokenType::COMMA));
		}
		expect(TokenType::RBRACE, "Expect '}' to close up import list");
		expect(TokenType::FROM, "Expect 'from' before import path");

		expect(TokenType::STRING, "Expect module path string");
		Token filepath = previous();
		expect(TokenType::SEMICOLON, "Expect ';' after import statement");

		return std::make_shared<ImportStmt>(keyword, std::move(symbols), filepath);
	}

	std::vector<StmtPtr> Parser::block()
	{
		std::vector<StmtPtr> statements;
		while (current_tok.type != TokenType::END_OF_FILE && !check(TokenType::RBRACE))
		{
			statements.push_back(declaration());
		}

		expect(TokenType::RBRACE, "Expected } at the end of a block");
		return statements;
	}

	ExprPtr Parser::expression()
	{
		return comma();
	}

	ExprPtr Parser::comma()
	{
		std::vector<ExprPtr> expressions;

		do
		{
			expressions.push_back(assignment());
		} while (match(TokenType::COMMA));

		return expressions.size() == 1 ? std::move(expressions[0]) : std::make_shared<PackExpr>(std::move(expressions));
	}

	ExprPtr Parser::assignment()
	{
		ExprPtr expr = ternary();

		if (match(TokenType::EQ, TokenType::PLUS_EQUAL, TokenType::MINUS_EQUAL, TokenType::MUL_EQUAL,
				  TokenType::DIV_EQUAL))
		{
			Token op = previous();
			ExprPtr rvalue = assignment(); // 这将允许 a = b = ... = 1;

			// 需要检查一下要去赋值的是不是一个变量(identifier)
			if (expr->exprType == ExprType::Variable)
			{
				Token identifier = std::static_pointer_cast<VariableExpr>(expr)->identifier;
				return std::make_shared<AssignmentExpr>(identifier, op, std::move(rvalue));
			}
			else if (expr->exprType == ExprType::Retrieve)
			{
				auto retrieval = std::static_pointer_cast<RetrieveExpr>(expr);

				if (retrieval->type == RetrieveExpr::OpType::DOT)
				{
					Token identifier = retrieval->identifier;
					return std::make_shared<SetExpr>(std::move(retrieval->holder), identifier, op, std::move(rvalue));
				}
				else
				{
					return std::make_shared<SetExpr>(std::move(retrieval->holder), std::move(retrieval->index), op, std::move(rvalue));
				}
			}

			throw ParsingError(expr->pos_start, rvalue->pos_end, "Invalid assignment target");
		}

		return expr;
	}

	ExprPtr Parser::ternary()
	{
		ExprPtr expr = logicOr();

		if (match(TokenType::QUESTION_MARK))
		{
			ExprPtr thenBranch = assignment();
			expect(TokenType::COLON, "Expect ':' after then branch for ternary expression");
			ExprPtr elseBranch = assignment();

			return std::make_shared<TernaryExpr>(std::move(expr), std::move(thenBranch), std::move(elseBranch));
		}

		return expr;
	}

	ExprPtr Parser::logicOr()
	{
		ExprPtr expr = logicAnd();
		while (match(TokenType::OR))
		{
			ExprPtr right = logicAnd();
			expr = std::make_shared<OrExpr>(std::move(expr), std::move(right));
		}

		return expr;
	}

	ExprPtr Parser::logicAnd()
	{
		ExprPtr expr = equality();
		while (match(TokenType::AND))
		{
			ExprPtr right = equality();
			expr = std::make_shared<AndExpr>(std::move(expr), std::move(right));
		}

		return expr;
	}

	ExprPtr Parser::equality()
	{
		return bin_op(&Parser::comparison, {TokenType::EQEQ, TokenType::BANGEQ}, &Parser::comparison);
	}

	ExprPtr Parser::comparison()
	{
		return bin_op(&Parser::term, {TokenType::GT, TokenType::GTE, TokenType::LT, TokenType::LTE}, &Parser::term);
	}

	ExprPtr Parser::term()
	{
		return bin_op(&Parser::factor, {TokenType::PLUS, TokenType::MINUS}, &Parser::factor);
	}

	ExprPtr Parser::factor()
	{
		return bin_op(&Parser::unary, {TokenType::MUL, TokenType::DIV, TokenType::MOD}, &Parser::unary);
	}

	ExprPtr Parser::unary()
	{
		if (match(TokenType::MINUS, TokenType::BANG))
		{
			Token op = previous();
			ExprPtr right = unary();
			return std::make_shared<UnaryExpr>(op, std::move(right));
		}

		return prefix();
	}

	ExprPtr Parser::prefix()
	{
		if (match(TokenType::PLUS_PLUS, TokenType::MINUS_MINUS))
		{
			Token op = previous();
			ExprPtr right = call();

			if (right->exprType == ExprType::Variable || right->exprType == ExprType::Retrieve)
			{
				if (op.type == TokenType::PLUS_PLUS)
					return std::make_shared<IncrementExpr>(std::move(right), IncrementExpr::Type::PREFIX);
				else if (op.type == TokenType::MINUS_MINUS)
					return std::make_shared<DecrementExpr>(std::move(right), DecrementExpr::Type::PREFIX);
			}
			else
			{
				throw ParsingError(right->pos_start, right->pos_end, "Can only '++' or '--' a variable");
			}
		}

		return postfix();
	}

	ExprPtr Parser::postfix()
	{
		ExprPtr expr = call();
		if (match(TokenType::PLUS_PLUS, TokenType::MINUS_MINUS))
		{
			Token op = previous();

			if (expr->exprType == ExprType::Variable || expr->exprType == ExprType::Retrieve)
			{
				if (op.type == TokenType::PLUS_PLUS)
					return std::make_shared<IncrementExpr>(std::move(expr), IncrementExpr::Type::POSTFIX);
				else if (op.type == TokenType::MINUS_MINUS)
					return std::make_shared<DecrementExpr>(std::move(expr), DecrementExpr::Type::POSTFIX);
			}
			else
			{
				throw ParsingError(expr->pos_start, expr->pos_end, "Can only '++' or '--' a variable");
			}
		}

		return expr;
	}

	ExprPtr Parser::call()
	{
		ExprPtr expr = primary();
		while (true)
		{
			if (match(TokenType::LPAREN))
				expr = fillArgs(std::move(expr));
			else if (match(TokenType::DOT))
			{
				expect(TokenType::IDENTIFIER, "Expect property name after '.'");
				Token prop = previous();
				expr = std::make_shared<RetrieveExpr>(std::move(expr), prop);
			}
			else if (match(TokenType::LBRACKET))
			{
				ExprPtr index = logicOr();
				expect(TokenType::RBRACKET, "Expect ']' to close up indexing");
				expr = std::make_shared<RetrieveExpr>(std::move(expr), std::move(index));
			}
			else
				break;
		}

		return expr;
	}

	ExprPtr Parser::primary()
	{
		if (match(TokenType::NUMBER, TokenType::STRING, TokenType::TRUE, TokenType::FALSE, TokenType::NIL))
		{
			Token pre = previous();
			return std::make_shared<LiteralExpr>(Object(pre), pre.pos_start, pre.pos_end);
		}
		else if (match(TokenType::IDENTIFIER))
		{
			return std::make_shared<VariableExpr>(previous());
		}
		else if (match(TokenType::LPAREN))
		{
			ExprPtr expr = expression();
			expect(TokenType::RPAREN, "Expected ')' for closure");
			return expr;
		}
		else if (match(TokenType::THIS))
		{
			return std::make_shared<ThisExpr>(previous());
		}
		else if (match(TokenType::SUPER))
		{
			Token keyword = previous();
			expect(TokenType::DOT, "Expected '.' to access super fields");
			expect(TokenType::IDENTIFIER, "Expected identifier after '.'");
			return std::make_shared<SuperExpr>(keyword, previous());
		}
		else if (match(TokenType::FUNC))
		{
			// aka Lambda
			return func_body();
		}
		else if (match(TokenType::LBRACKET))
		{
			return list_expr();
		}
		else
		{
			throw ParsingError(current_tok.pos_start, current_tok.pos_end, "Expected expression");
		}
	}

	ExprPtr Parser::list_expr()
	{
		Token lbracket = previous();

		std::vector<ExprPtr> args = arguments(TokenType::RBRACKET);

		expect(TokenType::RBRACKET, "Expect ']' to close up MetaList");
		Token rbracket = previous();

		return std::make_shared<ListExpr>(lbracket, std::move(args), rbracket);
	}

	ExprPtr Parser::bin_op(const std::function<ExprPtr(Parser *)> &funcA, std::initializer_list<TokenType> ops,
						   const std::function<ExprPtr(Parser *)> &funcB)
	{
		ExprPtr expr = funcA(this);
		while (match(ops))
		{
			Token op = previous();
			ExprPtr right = funcB(this);
			expr = std::make_shared<BinaryExpr>(std::move(expr), std::move(right), op);
		}

		return expr;
	}

	std::vector<ExprPtr> Parser::arguments(TokenType ending)
	{
		std::vector<ExprPtr> arguments;
		if (!check(ending))
		{
			do
			{
				arguments.push_back(ternary());
			} while (match(TokenType::COMMA));
		}

		return arguments;
	}

	ExprPtr Parser::fillArgs(ExprPtr expr)
	{
		std::vector<ExprPtr> args = arguments(TokenType::RPAREN);

		expect(TokenType::RPAREN, "Expect ')' to close up argument list");

		return std::make_shared<CallExpr>(std::move(expr), std::move(args));
	}

	ExprPtr Parser::func_body()
	{
		expect(TokenType::LPAREN, "Expected '(' before parameter list");

		std::vector<Token> parameters;
		std::vector<ExprPtr> default_values;

		if (!check(TokenType::RPAREN))
		{
			size_t idx = 0;							// 记录当前参数下标
			size_t first_optional_idx = 0x7FFFFFFF; // 第一个带有默认值的参数位置
			size_t last_required_idx = 0;			// 记录最后一个必须参数位置
			Position start = current_tok.pos_start;

			do
			{
				expect(TokenType::IDENTIFIER, "Expected a parameter name");
				parameters.push_back(previous());

				// 默认值
				if (match(TokenType::EQ))
				{
					default_values.push_back(ternary());
					if (first_optional_idx > idx)
						first_optional_idx = idx;
				}
				else
				{
					last_required_idx = idx;
				}

				idx++;
			} while (match(TokenType::COMMA));

			if (last_required_idx > first_optional_idx)
			{
				throw ParsingError(start, current_tok.pos_end, "Optional parameters cannot precede required one.");
			}
		}

		expect(TokenType::RPAREN, "Expected ')' after parameter list");
		expect(TokenType::LBRACE, "Expected '{' before function body");

		std::vector<StmtPtr> body = block();

		return std::make_shared<LambdaExpr>(std::move(parameters), std::move(default_values), std::move(body));
	}

	void Parser::advance()
	{
		tok_idx++;
		if (tok_idx < tokens.size())
			current_tok = tokens[tok_idx];
	}

	void Parser::reverse(int step)
	{
		tok_idx -= step;
		if (tok_idx >= 0)
			current_tok = tokens[tok_idx];
		else
			tok_idx += step;
	}

	Token Parser::previous()
	{
		return tokens[tok_idx - 1];
	}

	bool Parser::check(const TokenType &type) const
	{
		return current_tok.type == type;
	}

	void Parser::expect(const TokenType &type, const std::string &error_message)
	{
		// 检查current_tok是否符合预期，若不符合则抛出对应错误信息
		if (!check(type))
			throw ParsingError(current_tok.pos_start, current_tok.pos_end, error_message);

		// 符合预期则跳过该Token
		advance();
	}

	bool Parser::match(const std::initializer_list<TokenType> &types)
	{
		if (std::any_of(std::begin(types), std::end(types), [&](TokenType type)
						{ return current_tok.type == type; }))
		{
			advance();
			return true;
		}

		return false;
	}

	void Parser::synchronize()
	{
		for (; current_tok.type != TokenType::END_OF_FILE;)
		{
			advance();

			if (match(TokenType::SEMICOLON))
				return;

			switch (current_tok.type)
			{
			case TokenType::VAR:
			case TokenType::IF:
			case TokenType::WHILE:
			case TokenType::FOR:
			case TokenType::FUNC:
			case TokenType::CLASS:
			case TokenType::RETURN:
				return;
			default:
				break;
			}
		}
	}

}