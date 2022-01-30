#pragma once

#include <memory>
#include <vector>
#include <map>
#include <optional>
#include "Lexer/Token.h"
#include "Common/typedefs.h"
#include "Interpreter/Object.h"

namespace CXX {

	// 对于AST节点采用访问者(visitor)设计模式
	// 从而满足开闭原则（对扩展开放，对修改关闭）

	class ExpressionStmt;

	class VarDeclarationStmt;

	class FuncDeclarationStmt;

	class ClassDeclarationStmt;

	class BlockStmt;

	class IfStmt;

	class WhileStmt;

	class ForStmt;

	class BreakStmt;

	class ContinueStmt;

	class ReturnStmt;

	class ImportStmt;

	// 这是一个用vector存储多个Stmt的节点，典型的如: var a,b,c;
	// PackStmt中将存储三个varDeclarationStmt
	class PackStmt;

	enum class StmtType
	{
		Expression,
		VarDecl,
		FuncDecl,
		ClassDecl,
		Block,
		If,
		While,
		For,
		Break,
		Continue,
		Return,
		Import,
		Pack
	};

	struct StmtVisitor
	{
		virtual ~StmtVisitor() = default;

		virtual void visit(const ExpressionStmt* expressionStmt) = 0;

		virtual void visit(const VarDeclarationStmt* varStmt) = 0;

		virtual void visit(std::shared_ptr<FuncDeclarationStmt> funcStmt) = 0;

		virtual void visit(const ClassDeclarationStmt* classStmt) = 0;

		virtual void visit(const BlockStmt* blockStmt) = 0;

		virtual void visit(const IfStmt* ifStmt) = 0;

		virtual void visit(const WhileStmt* whileStmt) = 0;

		virtual void visit(const ForStmt* forStmt) = 0;

		virtual void visit(const BreakStmt* breakStmt) = 0;

		virtual void visit(const ContinueStmt* continueStmt) = 0;

		virtual void visit(const ReturnStmt* returnStmt) = 0;

		virtual void visit(const ImportStmt* importStmt) = 0;

		virtual void visit(const PackStmt* packStmt) = 0;
	};

	class Stmt
	{
	public:
		virtual ~Stmt() = default;

		virtual void accept(StmtVisitor& visitor) = 0;

		[[nodiscard]] virtual std::string to_string() const = 0;

		void set_pos(const Position& start, const Position& end)
		{
			this->pos_start = start;
			this->pos_end = end;
		}

	public:
		Position pos_start;
		Position pos_end;
		StmtType stmtType;
	};

	class ExpressionStmt : public Stmt
	{
	public:
		explicit ExpressionStmt(ExprPtr expr);

		void accept(StmtVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		ExprPtr expr;
	};

	class VarDeclarationStmt : public Stmt
	{
	public:
		VarDeclarationStmt(const Token& identifier, std::optional<ExprPtr> expr);

		void accept(StmtVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		Token identifier;

		// 你可以只声明而不赋值
		std::optional<ExprPtr> expr;
	};

	class FuncDeclarationStmt : public Stmt, public std::enable_shared_from_this<FuncDeclarationStmt>
	{
	public:
		FuncDeclarationStmt(const Token& name, std::vector<Token> params, std::vector<ExprPtr> default_values, std::vector<StmtPtr> body);

		void accept(StmtVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		Token name;
		std::vector<Token> params;
		std::vector<ExprPtr> default_values;
		std::vector<StmtPtr> body;
	};

	class VariableExpr; // 类可以继承自另一个类

	class ClassDeclarationStmt : public Stmt
	{
	public:
		ClassDeclarationStmt(const Token& identifier, std::vector<std::shared_ptr<FuncDeclarationStmt>> methods,
			std::optional<std::shared_ptr<VariableExpr>> superclass = std::nullopt);

		void accept(StmtVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		Token name;
		std::vector<std::shared_ptr<FuncDeclarationStmt>> methods;
		// 仅支持单继承
		std::optional<std::shared_ptr<VariableExpr>> superClass;
	};

	class BlockStmt : public Stmt
	{
	public:
		explicit BlockStmt(std::vector<StmtPtr> statements);

		void accept(StmtVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		std::vector<StmtPtr> statements;
	};

	class IfStmt : public Stmt
	{
	public:
		IfStmt(ExprPtr condition, StmtPtr thenBranch, std::optional<StmtPtr> elseBranch);

		void accept(StmtVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		ExprPtr condition;
		StmtPtr thenBranch;
		std::optional<StmtPtr> elseBranch;
	};

	class WhileStmt : public Stmt
	{
	public:
		WhileStmt(ExprPtr condition, StmtPtr body);

		void accept(StmtVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		ExprPtr condition;
		StmtPtr body;
	};

	class ForStmt : public Stmt
	{
	public:
		ForStmt(std::optional<StmtPtr> initializer, std::optional<ExprPtr> condition, std::optional<ExprPtr> increment,
			StmtPtr body);

		void accept(StmtVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		std::optional<StmtPtr> initializer;
		std::optional<ExprPtr> condition;
		std::optional<ExprPtr> increment;
		StmtPtr body;
	};

	class BreakStmt : public Stmt
	{
	public:
		explicit BreakStmt(const Token& keyword);

		void accept(StmtVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		// 实际上这个类完全不需要任何成员
		// 但是为了保证Position Tracking的完整性，我们给它一个keyword
		Token keyword;
	};

	class BreakFlag
	{
		// throw一个Break标志是最直接的方式
		// try catch性能效率很低，但是Break和Continue不是常触发语句
		// 可以接受
	};

	class ContinueStmt : public Stmt
	{
	public:
		explicit ContinueStmt(const Token& keyword);

		void accept(StmtVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		// 同Break，实际上这个类完全不需要任何成员
		// 但是为了保证Position Tracking的完整性，我们给它一个keyword
		Token keyword;
	};

	class ContinueFlag
	{
		// throw一个Continue标志是最直接的方式
		// try catch性能效率很低，但是Break和Continue不是常触发语句
		// 可以接受
	};

	class ReturnStmt : public Stmt
	{
	public:
		explicit ReturnStmt(const Token& keyword, std::optional<ExprPtr> expr);

		void accept(StmtVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		Token keyword;
		std::optional<ExprPtr> expr;
	};

	class ImportStmt : public Stmt
	{
	public:
		ImportStmt(const Token& keyword, std::map<Token, std::optional<Token>> symbols, const Token& filepath);

		void accept(StmtVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		Token keyword;
		std::map<Token, std::optional<Token>> symbols;
		Token filepath;
	};

	class PackStmt : public Stmt
	{
	public:
		PackStmt(std::vector<StmtPtr> stmts);

		void accept(StmtVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		std::vector<StmtPtr> statements;
	};

	class ErrorStmt : public Stmt // 仅用于存储ParseError的位置信息
	{
	public:
		ErrorStmt(const Position& start, const Position& end) { set_pos(start, end); }

		void accept(StmtVisitor& visitor) override {}

		[[nodiscard]] std::string to_string() const override { return "ErrorStmt"; }
	};

}