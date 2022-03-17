#pragma once
#include "Common/typedefs.h"
#include "Parser/Expr.h"
#include "Parser/Stmt.h"
#include "Interpreter/Object.h"
#include "Interpreter/Context.h"
#include "Interpreter/Module.h"
#include "Interpreter/RuntimeError.h"

namespace CXX {

	class Interpreter : public ExprVisitor, public StmtVisitor
	{
	public:
		Interpreter();

		void interpret(const std::vector<StmtPtr>& statements);

		void interpret(std::vector<StmtPtr>&& statements);

		Object interpret(Expr* expr);

		void execute(Stmt* pStmt);

		Object getReturn();

	public:
		void visit(const ExpressionStmt* expressionStmt) override;

		void visit(const VarDeclarationStmt* varStmt) override;

		void visit(std::shared_ptr<FuncDeclarationStmt> funcDeclStmt) override;

		void visit(const ClassDeclarationStmt* classDeclStmt) override;

		void visit(const BlockStmt* blockStmt) override;

		void visit(const IfStmt* ifStmt) override;

		void visit(const WhileStmt* whileStmt) override;

		void visit(const ForStmt* forStmt) override;

		void visit(const BreakStmt* breakStmt) override;

		void visit(const ContinueStmt* continueStmt) override;

		void visit(const ReturnStmt* returnStmt) override;

		void visit(const ImportStmt* importStmt) override;

		void visit(const PackStmt* packStmt) override;

		Object visit(const BinaryExpr* binaryExpr) override;

		Object visit(const UnaryExpr* unaryExpr) override;

		Object visit(const LiteralExpr* literalExpr) override;

		Object visit(const VariableExpr* variableExpr) override;

		Object visit(const AssignmentExpr* assignmentExpr) override;

		Object visit(const TernaryExpr* ternaryExpr) override;

		Object visit(std::shared_ptr<LambdaExpr> lambdaExpr) override;

		Object visit(const OrExpr* orExpr) override;

		Object visit(const AndExpr* andExpr) override;

		Object visit(const IncrementExpr* incrementExpr) override;

		Object visit(const DecrementExpr* decrementExpr) override;

		Object visit(const CallExpr* callExpr) override;

		Object visit(const RetrieveExpr* retrieveExpr) override;

		Object visit(const SetExpr* setExpr) override;

		Object visit(const ThisExpr* thisExpr) override;

		Object visit(const SuperExpr* superExpr) override;

		Object visit(const ListExpr* listExpr) override;

		Object visit(const PackExpr* packExpr) override;

	public:
		ContextPtr presetContext; // 此处用来存储内置函数，内置变量，
		ContextPtr globalContext; // 此处用来存储全局变量
		ContextPtr context;		  // 指向运行时的"当前环境"

		// 记录代码执行过程中的返回值
		// TEST：我没有发现返回值栈大小超过1的情况，因此改用optional
		std::optional<Object> m_returns;

		// filepath : module
		std::unordered_map<std::string, std::shared_ptr<Module>> m_modules;

	public:
		Callable* currentFunction{ nullptr }; // 指向当前在运行的函数/构造函数
		bool replEcho{ false };

	private:
		void loadPresetEnvironment();

		Object& lookupVariable(const Token& identifier, int depth);

		Object handleAssign(const Object& lhs, const Object& rhs, TokenType op);

		Object& listAt(const Object& list, const Object& index);

		std::shared_ptr<Module> loadModule(const Token& filepath);
	};

}