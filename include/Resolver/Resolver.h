#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Common/typedefs.h"
#include "Parser/Expr.h"
#include "Parser/Stmt.h"
#include "Resolver/ResolvingError.h"
#include "Interpreter/Object.h"

namespace CXX {

	class Token;

	// Resolver作用于Parser之后，Interpreter之前，属于语义分析的过程。
	// 它几乎是一个简化版的Interpreter，意义在于理清各个Statement之间包含的变量，
	// 完成变量绑定；同时也可以避免一个变量被反复解析（例如在一个循环体里）。
	// Resolver是静态分析，而Interpreter是动态分析。
	// 这意味着Resolver不会产生任何效果，也不会真的去执行控制语句
	// 更多信息请搜索：静态作用域、动态闭包
	class Resolver : public ExprVisitor, StmtVisitor
	{
	public:
		bool resolve(const std::vector<StmtPtr>& stmts);

		void resolve(Stmt* stmt);

		void resolve(Expr* expr);

		Object visit(const BinaryExpr* binaryExpr) override;

		Object visit(const UnaryExpr* unaryExpr) override;

		Object visit(const LiteralExpr* literalExpr) override;

		Object visit(const VariableExpr* variableExpr) override;

		Object visit(const AssignmentExpr* assignmentExpr) override;

		Object visit(const TernaryExpr* ternaryExpr) override;

		Object visit(const OrExpr* orExpr) override;

		Object visit(const AndExpr* andExpr) override;

		Object visit(const CallExpr* functionCallExpr) override;

		Object visit(const RetrieveExpr* retrieveExpr) override;

		Object visit(const SetExpr* setExpr) override;

		Object visit(const IncrementExpr* incrementExpr) override;

		Object visit(const DecrementExpr* decrementExpr) override;

		Object visit(std::shared_ptr<LambdaExpr> lambdaExpr) override;

		Object visit(const ThisExpr* thisExpr) override;

		Object visit(const SuperExpr* superExpr) override;

		Object visit(const ListExpr* listExpr) override;

		Object visit(const PackExpr* packExpr) override;

		void visit(const ExpressionStmt* expressionStmt) override;

		void visit(const VarDeclarationStmt* varStmt) override;

		void visit(const BlockStmt* blockStmt) override;

		void visit(const IfStmt* ifStmt) override;

		void visit(const WhileStmt* whileStmt) override;

		void visit(const BreakStmt* breakStmt) override;

		void visit(const ContinueStmt* continueStmt) override;

		void visit(const ForStmt* forStmt) override;

		void visit(std::shared_ptr<FuncDeclarationStmt> funcDeclStmt) override;

		void visit(const ClassDeclarationStmt* classDeclStmt) override;

		void visit(const ReturnStmt* returnStmt) override;

		void visit(const ImportStmt* importStmt) override;

		void visit(const PackStmt* packStmt) override;

	private:
		// 实际上，这些作用域应该是栈结构，我们这里用vector手动模拟
		// 哈希表：表示一个变量在此作用域内已经初始化过与否
		std::vector<std::unordered_map<std::string, bool>> scopes;

		// 用于记录程序执行过程中的循环层数
		// 防止错误使用break和continue
		int loopLayer = 0;

		// 用于记录程序执行过程中的函数栈
		// 防止在非函数体内return
		enum class FunctionType
		{
			NONE,
			FUNCTION,
			METHOD,		 // method专指类成员函数
			INITIALIZER, // 类构造函数，不允许返回值
		};
		FunctionType currentFunction = FunctionType::NONE;

		// 用于记录当前代码段是否处于Class内部
		// 防止在非类成员函数中使用this
		enum class ClassType
		{
			NONE,
			CLASS,
			SUBCLASS
		};
		ClassType currentClass = ClassType::NONE;

	private:
		int resolveLocal(const Token& name);

		void resolveFunction(const FuncDeclarationStmt* functionStmt, FunctionType type);

		void resolveFunction(const LambdaExpr* lambdaExpr);

		void beginScope();

		void endScope();

		void define(const Token& name);

		void declare(const Token& name);
	};

}