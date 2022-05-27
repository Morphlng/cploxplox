#pragma once
#include "Common/typedefs.h"
#include "Parser/Expr.h"
#include "Parser/Stmt.h"
#include "Interpreter/Object.h"
#include "xmlTranspiler/Variable.h"
#include <unordered_map>

namespace CXX
{
	class Transpiler : public ExprVisitor, public StmtVisitor
	{
	public:
		Transpiler() = default;
		~Transpiler() = default;

		std::string &transpile(const std::vector<StmtPtr> &statements);

	public:
		// ExprBlock will always close up with </block>.
		// StmtBlock may need to use <next> to splice another block,
		// so we close stmtblock outside of visit function

		void visit(const ExpressionStmt *expressionStmt) override;

		void visit(const VarDeclarationStmt *varStmt) override;

		void visit(std::shared_ptr<FuncDeclarationStmt> funcDeclStmt) override;

		void visit(const ClassDeclarationStmt *classDeclStmt) override;

		void visit(const BlockStmt *blockStmt) override;

		void visit(const IfStmt *ifStmt) override;

		void visit(const WhileStmt *whileStmt) override;

		void visit(const ForStmt *forStmt) override;

		void visit(const BreakStmt *breakStmt) override;

		void visit(const ContinueStmt *continueStmt) override;

		void visit(const ReturnStmt *returnStmt) override;

		void visit(const ImportStmt *importStmt) override;

		void visit(const PackStmt *packStmt) override;

		Object visit(const BinaryExpr *binaryExpr) override;

		Object visit(const UnaryExpr *unaryExpr) override;

		Object visit(const LiteralExpr *literalExpr) override;

		Object visit(const VariableExpr *variableExpr) override;

		Object visit(const AssignmentExpr *assignmentExpr) override;

		Object visit(const TernaryExpr *ternaryExpr) override;

		Object visit(std::shared_ptr<LambdaExpr> lambdaExpr) override;

		Object visit(const OrExpr *orExpr) override;

		Object visit(const AndExpr *andExpr) override;

		Object visit(const IncrementExpr *incrementExpr) override;

		Object visit(const DecrementExpr *decrementExpr) override;

		Object visit(const CallExpr *callExpr) override;

		Object visit(const RetrieveExpr *retrieveExpr) override;

		Object visit(const SetExpr *setExpr) override;

		Object visit(const ThisExpr *thisExpr) override;

		Object visit(const SuperExpr *superExpr) override;

		Object visit(const ListExpr *listExpr) override;

		Object visit(const PackExpr *packExpr) override;

	private:
		void translate(Stmt *stmt);
		void translate(Expr *expr);
		void visitRet(const ReturnStmt *returnStmt, const std::string &value_name = "RETURN");
		void newScope(const std::vector<StmtPtr> &statements);

		std::string varID(const VariableExpr *variableExpr);
		std::string varID(const std::string &varName);

	private:
		std::string xmlCode;
		std::unordered_map<std::string, std::string> variableDB;
		std::unordered_map<std::string, Function> functionDB;
	};

}