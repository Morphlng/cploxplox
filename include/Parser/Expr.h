#pragma once

#include <memory>
#include <vector>
#include "Common/typedefs.h"
#include "Common/Position.h"
#include "Lexer/Token.h"
#include "Interpreter/Object.h"

namespace CXX {

	// 对于AST节点采用访问者(visitor)设计模式
	// 从而满足开闭原则（对扩展开放，对修改关闭）

	class BinaryExpr;

	class UnaryExpr;

	class LiteralExpr;

	class VariableExpr;

	class AssignmentExpr; // 赋值语句：a=1

	class TernaryExpr; // 三元表达式 a ? b : c

	class OrExpr; // 把"与/或"单独拿出来，是因为他们的处理和普通的二元运算不同

	class AndExpr; // 例如，有一个为否，与的结果就是否；所以单独处理比较方便

	class IncrementExpr; // ++

	class DecrementExpr; // --

	class CallExpr; // call函数

	class RetrieveExpr; // 获取类成员，或列表项

	// 修改类成员，或列表项
	// 与Assign的不同在于，Set的对象一般是运行时确认
	// 故而在Resolver中不需要对该Expr进行解析
	class SetExpr;

	class ThisExpr; // 表明接下来索取的是类数据成员

	class SuperExpr; // 表明从父类中寻找数据成员

	class LambdaExpr;

	class ListExpr;

	class PackExpr; // 同理PackStmt，这是一个vector<ExprPtr>

	enum class ExprType
	{
		Binary,
		Unary,
		Literal,
		Variable,
		Assignment,
		Ternary,
		Or,
		And,
		Increment,
		Decrement,
		Call,
		Retrieve,
		Set,
		This,
		Super,
		Lambda,
		List,
		Pack
	};

	// 每个Expr的解释结果应为一个Object
	struct ExprVisitor
	{
		virtual ~ExprVisitor() = default;

		virtual Object visit(const BinaryExpr* binaryExpr) = 0;

		virtual Object visit(const UnaryExpr* unaryExpr) = 0;

		virtual Object visit(const LiteralExpr* literalExpr) = 0;

		virtual Object visit(const VariableExpr* varExpr) = 0;

		virtual Object visit(const AssignmentExpr* assignmentExpr) = 0;

		virtual Object visit(const TernaryExpr* ternaryExpr) = 0;

		virtual Object visit(const OrExpr* orExpr) = 0;

		virtual Object visit(const AndExpr* andExpr) = 0;

		virtual Object visit(const IncrementExpr* incrementExpr) = 0;

		virtual Object visit(const DecrementExpr* decrementExpr) = 0;

		virtual Object visit(const CallExpr* callExpr) = 0;

		virtual Object visit(const RetrieveExpr* accessExpr) = 0;

		virtual Object visit(const SetExpr* setExpr) = 0;

		virtual Object visit(std::shared_ptr<LambdaExpr> lambdaExpr) = 0;

		virtual Object visit(const ThisExpr* thisExpr) = 0;

		virtual Object visit(const SuperExpr* superExpr) = 0;

		virtual Object visit(const ListExpr* listExpr) = 0;

		virtual Object visit(const PackExpr* packExpr) = 0;
	};

	class Expr
	{
	public:
		virtual ~Expr() = default;

		virtual Object accept(ExprVisitor& visitor) = 0;

		[[nodiscard]] virtual std::string to_string() const = 0;

		void set_pos(const Position& start, const Position& end)
		{
			this->pos_start = start;
			this->pos_end = end;
		}

	public:
		Position pos_start;
		Position pos_end;
		ExprType exprType;
	};

	class BinaryExpr : public Expr
	{
	public:
		BinaryExpr(ExprPtr left, ExprPtr right, const Token& op);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		ExprPtr left;
		ExprPtr right;
		Token op;
	};

	class UnaryExpr : public Expr
	{
	public:
		UnaryExpr(const Token& op, ExprPtr expr);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		Token op;
		ExprPtr expr;
	};

	class LiteralExpr : public Expr
	{
	public:
		explicit LiteralExpr(Object value, const Position& pos_start = Position::preset,
			const Position& pos_end = Position::preset);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		Object value;
	};

	class VariableExpr : public Expr
	{
	public:
		explicit VariableExpr(const Token& identifier);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

		void resolve(int depth);

	public:
		Token identifier;
		int depth;
	};

	class AssignmentExpr : public Expr
	{
	public:
		AssignmentExpr(const Token& identifier, const Token& operation, ExprPtr value);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

		void resolve(int depth);

	public:
		Token identifier;
		Token operation;
		ExprPtr value;
		int depth;
	};

	class TernaryExpr : public Expr
	{
	public:
		TernaryExpr(ExprPtr ifExpr, ExprPtr thenBranch, ExprPtr elseBranch);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		ExprPtr expr;
		ExprPtr thenBranch;
		ExprPtr elseBranch;
	};

	class OrExpr : public Expr
	{
	public:
		OrExpr(ExprPtr left, ExprPtr right);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		ExprPtr left, right;
	};

	class AndExpr : public Expr
	{
	public:
		AndExpr(ExprPtr left, ExprPtr right);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		ExprPtr left, right;
	};

	class IncrementExpr : public Expr
	{
	public:
		enum class Type
		{
			POSTFIX,
			PREFIX
		};

		IncrementExpr(ExprPtr holder, IncrementExpr::Type type);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		ExprPtr holder; // 可以是Variable或Retrieve
		IncrementExpr::Type type;
	};

	class DecrementExpr : public Expr
	{
	public:
		enum class Type
		{
			POSTFIX,
			PREFIX
		};

		DecrementExpr(ExprPtr holder, Type type);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		ExprPtr holder; // 可以是Variable或Retrieve
		DecrementExpr::Type type;
	};

	class CallExpr : public Expr
	{
	public:
		CallExpr(ExprPtr callee, std::vector<ExprPtr> arguments);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		ExprPtr callee;
		std::vector<ExprPtr> arguments;
	};

	class RetrieveExpr : public Expr
	{
	public:
		enum class OpType
		{
			DOT,
			BRACKET
		};

		RetrieveExpr(ExprPtr expr, const Token& identifier, OpType type = OpType::DOT);

		RetrieveExpr(ExprPtr expr, ExprPtr index, OpType type = OpType::BRACKET);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		ExprPtr holder;
		Token identifier; // 从对象中取元素，使用identifier
		ExprPtr index;	  // 从列表中取元素，则为Number；否则应为string
		OpType type;
	};

	class SetExpr : public Expr
	{
		using OpType = RetrieveExpr::OpType;

	public:
		SetExpr(ExprPtr expr, const Token& identifier, const Token& operation, ExprPtr value,
			OpType type = OpType::DOT);

		SetExpr(ExprPtr expr, ExprPtr index, const Token& operation, ExprPtr value,
			OpType type = OpType::BRACKET);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		ExprPtr holder;
		Token identifier; // 从对象中取元素，使用identifier
		ExprPtr index;	  // 从列表中取元素，则为Number；否则应为string
		Token operation;  // +=、-=、*=、/=、=
		ExprPtr value;
		OpType type;
	};

	class LambdaExpr : public Expr, public std::enable_shared_from_this<LambdaExpr>
	{
	public:
		LambdaExpr(std::vector<Token> params, std::vector<ExprPtr> default_values, std::vector<StmtPtr> body);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		std::vector<Token> params;
		std::vector<ExprPtr> default_values;
		std::vector<StmtPtr> body;
	};

	class ThisExpr : public Expr
	{
	public:
		explicit ThisExpr(const Token& keyword);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

		void resolve(int depth);

	public:
		Token keyword;
		int depth;
	};

	class SuperExpr : public Expr
	{
	public:
		SuperExpr(const Token& keyword, const Token& identifier);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

		void resolve(int depth);

	public:
		Token keyword;
		Token identifier;
		int depth;
	};

	class ListExpr : public Expr
	{
	public:
		ListExpr(const Token& left, std::vector<ExprPtr> items, const Token& right);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		Token leftBracket;
		std::vector<ExprPtr> items;
		Token rightBracket;
	};

	class PackExpr : public Expr
	{
	public:
		PackExpr(std::vector<ExprPtr> exprs);

		Object accept(ExprVisitor& visitor) override;

		[[nodiscard]] std::string to_string() const override;

	public:
		std::vector<ExprPtr> expressions;
	};

}