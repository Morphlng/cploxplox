#include "Resolver/Resolver.h"
#include "Common/utils.h"
#include <filesystem>
#include <cstdlib>

namespace CXX
{

	bool Resolver::resolve(const std::vector<StmtPtr> &stmts)
	{
		for (auto const &stmt : stmts)
		{
			resolve(stmt.get());
		}

		return ErrorReporter::errorCount == 0;
	}

	Object Resolver::visit(const BinaryExpr *binaryExpr)
	{
		resolve(binaryExpr->left.get());
		resolve(binaryExpr->right.get());
		return Object();
	}

	Object Resolver::visit(const UnaryExpr *unaryExpr)
	{
		resolve(unaryExpr->expr.get());
		return Object();
	}

	Object Resolver::visit(const LiteralExpr *literalExpr)
	{
		return Object();
	}

	Object Resolver::visit(const VariableExpr *variableExpr)
	{
		const std::string &name = variableExpr->identifier.lexeme;

		if (!scopes.empty())
		{
			auto &nearest_scope = scopes.back();

			// 这里处理的情况是 var a = a;
			if (nearest_scope.find(name) != nearest_scope.end() && !nearest_scope.find(name)->second)
			{
				ErrorReporter::report(ResolvingError(variableExpr->pos_start, variableExpr->pos_end, "Can't init a variable with it self"));
				return Object();
			}
		}

		const_cast<VariableExpr *>(variableExpr)->resolve(resolveLocal(variableExpr->identifier));
		return Object();
	}

	Object Resolver::visit(const AssignmentExpr *assignmentExpr)
	{
		resolve(assignmentExpr->value.get());
		const_cast<AssignmentExpr *>(assignmentExpr)->resolve(resolveLocal(assignmentExpr->identifier));
		return Object();
	}

	Object Resolver::visit(const TernaryExpr *ternaryExpr)
	{
		resolve(ternaryExpr->expr.get());
		resolve(ternaryExpr->thenBranch.get());
		resolve(ternaryExpr->elseBranch.get());

		return Object();
	}

	Object Resolver::visit(const OrExpr *orExpr)
	{
		resolve(orExpr->left.get());
		resolve(orExpr->right.get());

		return Object();
	}

	Object Resolver::visit(const AndExpr *andExpr)
	{
		resolve(andExpr->left.get());
		resolve(andExpr->right.get());

		return Object();
	}

	Object Resolver::visit(const CallExpr *functionCallExpr)
	{
		resolve(functionCallExpr->callee.get());
		for (auto const &arg : functionCallExpr->arguments)
		{
			resolve(arg.get());
		}

		return Object();
	}

	Object Resolver::visit(const RetrieveExpr *retrieveExpr)
	{
		// Resolver只做静态分析，Retrieve属于运行时操作，因此没有绑定操作
		resolve(retrieveExpr->holder.get());
		if (retrieveExpr->index)
			resolve(retrieveExpr->index.get());

		return Object();
	}

	Object Resolver::visit(const SetExpr *setExpr)
	{
		resolve(setExpr->holder.get());
		if (setExpr->index)
			resolve(setExpr->index.get());

		resolve(setExpr->value.get());
		return Object();
	}

	Object Resolver::visit(const IncrementExpr *incrementExpr)
	{
		if (incrementExpr->holder->exprType == ExprType::Variable)
		{
			auto var = static_cast<VariableExpr *>(incrementExpr->holder.get());
			var->resolve(resolveLocal(var->identifier));
		}
		else
		{
			// 若不是变量，则为Retrieve操作，运行时操作Resolver不处理
			resolve(incrementExpr->holder.get());
		}

		return Object();
	}

	Object Resolver::visit(const DecrementExpr *decrementExpr)
	{
		if (decrementExpr->holder->exprType == ExprType::Variable)
		{
			auto var = static_cast<VariableExpr *>(decrementExpr->holder.get());
			var->resolve(resolveLocal(var->identifier));
		}
		else
		{
			// 若不是变量，则为Retrieve操作，运行时操作Resolver不处理
			resolve(decrementExpr->holder.get());
		}

		return Object();
	}

	Object Resolver::visit(std::shared_ptr<LambdaExpr> lambdaExpr)
	{
		resolveFunction(lambdaExpr.get());
		return Object();
	}

	Object Resolver::visit(const ThisExpr *thisExpr)
	{
		if (currentClass == ClassType::NONE)
		{
			ErrorReporter::report(ResolvingError(thisExpr->pos_start, thisExpr->pos_end, "\"this\" can only be used inside a class method"));
			return Object();
		}

		const_cast<ThisExpr *>(thisExpr)->resolve(resolveLocal(thisExpr->keyword));

		return Object();
	}

	Object Resolver::visit(const SuperExpr *superExpr)
	{
		if (currentClass != ClassType::SUBCLASS)
		{
			ErrorReporter::report(ResolvingError(superExpr->pos_start, superExpr->pos_end, "Cannot use 'super' outside of a subclass"));
			return Object();
		}

		const_cast<SuperExpr *>(superExpr)->resolve(resolveLocal(superExpr->keyword));

		return Object();
	}

	Object Resolver::visit(const ListExpr *listExpr)
	{
		for (auto &item : listExpr->items)
			resolve(item.get());

		return Object();
	}

	Object Resolver::visit(const PackExpr *packExpr)
	{
		for (auto const &expr : packExpr->expressions)
		{
			resolve(expr.get());
		}

		return Object();
	}

	void Resolver::visit(const ExpressionStmt *expressionStmt)
	{
		resolve(expressionStmt->expr.get());
	}

	void Resolver::visit(const VarDeclarationStmt *varStmt)
	{
		declare(varStmt->identifier);
		if (varStmt->expr)
		{
			resolve(varStmt->expr.value().get());
		}
		define(varStmt->identifier);
	}

	void Resolver::visit(const BlockStmt *blockStmt)
	{
		beginScope();
		resolve(blockStmt->statements);
		endScope();
	}

	void Resolver::visit(const IfStmt *ifStmt)
	{
		resolve(ifStmt->condition.get());
		resolve(ifStmt->thenBranch.get());
		if (ifStmt->elseBranch)
		{
			resolve(ifStmt->elseBranch.value().get());
		}
	}

	void Resolver::visit(const WhileStmt *whileStmt)
	{
		loopLayer++;

		resolve(whileStmt->condition.get());
		resolve(whileStmt->body.get());

		loopLayer--;
	}

	void Resolver::visit(const ForStmt *forStmt)
	{
		loopLayer++;

		beginScope();
		if (forStmt->initializer)
			resolve(forStmt->initializer.value().get());

		if (forStmt->condition)
			resolve(forStmt->condition.value().get());

		if (forStmt->increment)
			resolve(forStmt->increment.value().get());

		resolve(forStmt->body.get());
		endScope();

		loopLayer--;
	}

	void Resolver::visit(const BreakStmt *breakStmt)
	{
		if (loopLayer == 0)
		{
			return ErrorReporter::report(ResolvingError(breakStmt->pos_start, breakStmt->pos_end, "'break' must be inside a loop"));
		}
	}

	void Resolver::visit(const ContinueStmt *continueStmt)
	{
		if (loopLayer == 0)
		{
			return ErrorReporter::report(ResolvingError(continueStmt->pos_start, continueStmt->pos_end, "'continue' must be inside a loop"));
		}
	}

	void Resolver::visit(std::shared_ptr<FuncDeclarationStmt> funcDeclStmt)
	{
		declare(funcDeclStmt->name);
		define(funcDeclStmt->name);
		resolveFunction(funcDeclStmt.get(), FunctionType::FUNCTION);
	}

	void Resolver::visit(const ReturnStmt *returnStmt)
	{
		if (currentFunction == FunctionType::NONE)
		{
			return ErrorReporter::report(ResolvingError(returnStmt->pos_start, returnStmt->pos_end, "'return' must be inside a function"));
		}

		if (returnStmt->expr)
		{
			if (currentFunction == FunctionType::INITIALIZER)
			{
				return ErrorReporter::report(ResolvingError(returnStmt->pos_start, returnStmt->pos_end,
															"Can't 'return' non-nil value from an initializer"));
			}

			resolve(returnStmt->expr.value().get());
		}
	}

	void Resolver::visit(const ImportStmt *importStmt)
	{
		namespace fs = std::filesystem;
		auto filepath = fs::path(importStmt->filepath.lexeme);
		bool existed = false;

		if (!filepath.has_extension())
		{
			filepath.concat(".lox");
		}

		if (!filepath.is_absolute())
		{
			const char *loxenv = std::getenv("LOXLIB");
			if (!loxenv)
			{
				filepath = fs::absolute(filepath);
				existed = fs::exists(filepath);
			}
			else
			{
				auto folders = split(loxenv, ";");
				for (auto &folder : folders)
				{
					auto tmpPath = fs::path(folder) / filepath;

					if (existed = fs::exists(tmpPath))
					{
						filepath = tmpPath;
						break;
					}
				}
			}
		}
		else
		{
			existed = fs::exists(filepath);
		}

		if (!existed)
		{
			return ErrorReporter::report(ResolvingError(importStmt->filepath.pos_start, importStmt->filepath.pos_end,
														"Invalid import path"));
		}

		const_cast<ImportStmt *>(importStmt)->filepath.lexeme = filepath.string();

		for (auto &[name, _] : importStmt->symbols)
		{
			declare(name);
			define(name);
		}
	}

	void Resolver::visit(const PackStmt *packStmt)
	{
		for (auto const &stmt : packStmt->statements)
		{
			resolve(stmt.get());
		}
	}

	void Resolver::visit(const ClassDeclarationStmt *classDeclStmt)
	{
		ClassType enclosing = currentClass;
		currentClass = ClassType::CLASS;

		declare(classDeclStmt->name);
		define(classDeclStmt->name);

		bool defineSuper{false};
		if (defineSuper = classDeclStmt->superClass.has_value())
		{
			currentClass = ClassType::SUBCLASS;
			auto ptr = classDeclStmt->superClass.value().get();
			if (ptr->identifier.lexeme == classDeclStmt->name.lexeme)
			{
				return ErrorReporter::report(ResolvingError(classDeclStmt->pos_start, ptr->pos_end, "A Class can't derived from itself"));
			}
			resolve(ptr);
		}

		beginScope();
		scopes.back().emplace("this", true);
		if (defineSuper)
			scopes.back().emplace("super", true);

		// 利用析构函数保证endScope运行
		Finally task{[&]()
					 {endScope(); currentClass = enclosing; }};

		for (auto &method : classDeclStmt->methods)
		{
			if (method->name.lexeme == "init")
				resolveFunction(method.get(), FunctionType::INITIALIZER);
			else if (method->name.lexeme == "__del__")
			{
				if (method->params.size() != 0)
				{
					return ErrorReporter::report(ResolvingError(method->params.front().pos_start, method->params.back().pos_end, "Destructor shouldn't take arguments"));
				}

				resolveFunction(method.get(), FunctionType::METHOD);
			}
			else
				resolveFunction(method.get(), FunctionType::METHOD);
		}
	}

	void Resolver::resolveFunction(const FuncDeclarationStmt *functionStmt, FunctionType type)
	{
		FunctionType enclosing = currentFunction;
		currentFunction = type;

		beginScope();
		for (auto &param : functionStmt->params)
		{
			declare(param);
			define(param);
		}
		resolve(functionStmt->body);
		endScope();
		currentFunction = enclosing;
	}

	void Resolver::resolveFunction(const LambdaExpr *lambdaExpr)
	{
		FunctionType enclosing = currentFunction;
		currentFunction = FunctionType::FUNCTION;

		beginScope();
		for (auto &param : lambdaExpr->params)
		{
			declare(param);
			define(param);
		}
		resolve(lambdaExpr->body);
		endScope();
		currentFunction = enclosing;
	}

	void Resolver::resolve(Stmt *stmt)
	{
		stmt->accept(*this);
	}

	void Resolver::resolve(Expr *expr)
	{
		expr->accept(*this);
	}

	int Resolver::resolveLocal(const Token &name)
	{
		int totalLength = scopes.size();
		for (int dist = totalLength - 1; dist >= 0; dist--)
		{
			if (scopes[dist].find(name.lexeme) != scopes[dist].end())
			{
				// 计算出该变量所处作用域距离当前表达式有几"跳"
				return totalLength - dist - 1;
			}
		}

		// 如果没有找到，则说明该变量为全局变量
		// Resolver不处理全局变量
		return -1;
	}

	void Resolver::beginScope()
	{
		scopes.emplace_back(std::unordered_map<std::string, bool>{});
	}

	void Resolver::endScope()
	{
		scopes.pop_back();
	}

	void Resolver::define(const Token &name)
	{
		if (scopes.empty())
			return;

		scopes.back()[name.lexeme] = true;
	}

	void Resolver::declare(const Token &name)
	{
		if (scopes.empty())
			return;

		// 此处可以用来检查是否有变量重定义
		// 我在此先允许，因为我认为这不会造成什么问题
		// 但是这种代码不被推荐，很可能是误定义
		scopes.back()[name.lexeme] = false;
	}

}