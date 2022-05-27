#include "Common/utils.h"
#include "Parser/Expr.h"
#include "Parser/Stmt.h"

namespace CXX {

	BinaryExpr::BinaryExpr(ExprPtr left, ExprPtr right, const Token& op) : left(std::move(left)), right(std::move(right)), op(op)
	{
		this->exprType = ExprType::Binary;
		set_pos(this->left->pos_start, this->right->pos_end);
	}

	Object BinaryExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string BinaryExpr::to_string() const
	{
		return format("BinaryExpr: [left:%s, op:%s, right:%s]", left->to_string().c_str(), op.to_string().c_str(),
			right->to_string().c_str());
	}

	UnaryExpr::UnaryExpr(const Token& op, ExprPtr expr) : op(op), expr(std::move(expr))
	{
		this->exprType = ExprType::Unary;
		set_pos(this->op.pos_start, this->expr->pos_end);
	}

	Object UnaryExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string UnaryExpr::to_string() const
	{
		return format("UnaryExpr: [op:%s, expr:%s]", op.to_string().c_str(), expr->to_string().c_str());
	}

	LiteralExpr::LiteralExpr(Object value, const Position& pos_start, const Position& pos_end) : value(std::move(value))
	{
		this->exprType = ExprType::Literal;
		set_pos(pos_start, pos_end);
	}

	Object LiteralExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string LiteralExpr::to_string() const
	{
		return format("Literal: %s", value.to_string().c_str());
	}

	VariableExpr::VariableExpr(const Token& identifier) : identifier(identifier), depth(-1)
	{
		this->exprType = ExprType::Variable;
		set_pos(this->identifier.pos_start, this->identifier.pos_end);
	}

	Object VariableExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string VariableExpr::to_string() const
	{
		return format("VariableExpr: %s", identifier.to_string().c_str());
	}

	void VariableExpr::resolve(int depth)
	{
		this->depth = depth;
	}

	AssignmentExpr::AssignmentExpr(const Token& identifier, const Token& operation, ExprPtr value) : identifier(identifier),
		operation(operation),
		value(std::move(value)),
		depth(-1)
	{
		this->exprType = ExprType::Assignment;
		set_pos(this->identifier.pos_start, this->value->pos_end);
	}

	Object AssignmentExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string AssignmentExpr::to_string() const
	{
		return format("AssignExpr: [%s %s %s]", identifier.lexeme.c_str(), operation.lexeme.c_str(),
			value->to_string().c_str());
	}

	void AssignmentExpr::resolve(int depth)
	{
		this->depth = depth;
	}

	TernaryExpr::TernaryExpr(ExprPtr ifExpr, ExprPtr thenBranch, ExprPtr elseBranch) : expr(std::move(ifExpr)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch))
	{
		this->exprType = ExprType::Ternary;
		set_pos(this->expr->pos_start, this->elseBranch->pos_end);
	}

	Object TernaryExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string TernaryExpr::to_string() const
	{
		return format("%s ? %s : %s", expr->to_string().c_str(), thenBranch->to_string().c_str(), elseBranch->to_string().c_str());
	}

	OrExpr::OrExpr(ExprPtr left, ExprPtr right) : left(std::move(left)), right(std::move(right))
	{
		this->exprType = ExprType::Or;
		set_pos(this->left->pos_start, this->right->pos_end);
	}

	Object OrExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string OrExpr::to_string() const
	{
		return format("OrExpr: [%s or %s]", left->to_string().c_str(), right->to_string().c_str());
	}

	AndExpr::AndExpr(ExprPtr left, ExprPtr right) : left(std::move(left)), right(std::move(right))
	{
		this->exprType = ExprType::And;
		set_pos(this->left->pos_start, this->right->pos_end);
	}

	Object AndExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string AndExpr::to_string() const
	{
		return format("AndExpr: [%s and %s]", left->to_string().c_str(), right->to_string().c_str());
	}

	IncrementExpr::IncrementExpr(ExprPtr holder, IncrementExpr::Type type) : holder(std::move(holder)), type(type)
	{
		this->exprType = ExprType::Increment;
		set_pos(this->holder->pos_start, this->holder->pos_end);
	}

	Object IncrementExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string IncrementExpr::to_string() const
	{
		if (type == Type::PREFIX)
			return format("IncrementExpr: ++%s", holder->to_string().c_str());
		else
			return format("IncrementExpr: %s++", holder->to_string().c_str());
	}

	DecrementExpr::DecrementExpr(ExprPtr holder, DecrementExpr::Type type) : holder(std::move(holder)), type(type)
	{
		this->exprType = ExprType::Decrement;
		set_pos(this->holder->pos_start, this->holder->pos_end);
	}

	Object DecrementExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string DecrementExpr::to_string() const
	{
		if (type == Type::PREFIX)
			return format("DecrementExpr: --%s", holder->to_string().c_str());
		else
			return format("DecrementExpr: %s--", holder->to_string().c_str());
	}

	CallExpr::CallExpr(ExprPtr callee, std::vector<ExprPtr> arguments) : callee(std::move(callee)), arguments(std::move(arguments))
	{
		this->exprType = ExprType::Call;
		if (this->arguments.empty())
		{
			set_pos(this->callee->pos_start, this->callee->pos_end);
		}
		else
		{
			set_pos(this->callee->pos_start, this->arguments.back()->pos_end);
		}
	}

	Object CallExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string CallExpr::to_string() const
	{
		std::string result = format("CallExpr: %s(", callee->to_string().c_str());
		;
		if (this->arguments.empty())
		{
			result.push_back(')');
		}
		else
		{
			for (auto& arg : arguments)
			{
				result += arg->to_string();
				result += ",";
			}
			char& end = result.back();
			end = ')';
		}
		return result;
	}

	LambdaExpr::LambdaExpr(std::vector<Token> params, std::vector<ExprPtr> default_values, std::vector<StmtPtr> body) : params(std::move(params)), default_values(std::move(default_values)), body(std::move(body))
	{
		this->exprType = ExprType::Lambda;
		this->pos_start = this->params.empty() ? (this->body.empty() ? Position::preset : this->body.front()->pos_start)
			: this->params.front().pos_start;
		this->pos_end = this->body.empty() ? (this->params.empty() ? Position::preset : this->params.back().pos_end)
			: this->body.back()->pos_end;
	}

	Object LambdaExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(shared_from_this());
	}

	std::string LambdaExpr::to_string() const
	{
		std::string result = "Lambda (";
		if (!params.empty())
		{
			for (auto& param : params)
			{
				result += param.lexeme;
				result.push_back(',');
			}
			result.pop_back();
		}

		result += ")\n{\n";
		for (auto& stmt : body)
		{
			result += stmt->to_string();
			result.push_back('\n');
		}
		result.push_back('}');

		return result;
	}

	RetrieveExpr::RetrieveExpr(ExprPtr expr, const Token& identifier, OpType type) : holder(std::move(expr)),
		identifier(identifier),
		type(type)
	{
		this->exprType = ExprType::Retrieve;
		set_pos(this->holder->pos_start, this->identifier.pos_end);
	}

	RetrieveExpr::RetrieveExpr(ExprPtr expr, ExprPtr index, OpType type) : holder(std::move(expr)), index(std::move(index)), type(type)
	{
		this->exprType = ExprType::Retrieve;
		set_pos(this->holder->pos_start, this->index->pos_end);
	}

	Object RetrieveExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string RetrieveExpr::to_string() const
	{
		std::string form;
		switch (type)
		{
		case OpType::DOT:
			form = "Retrieve: %s.%s";
			break;

		case OpType::BRACKET:
			form = "Retrieve: %s[%s]";
			break;

		default:
			break;
		}

		return format(form, holder->to_string().c_str(), identifier.lexeme.c_str());
	}

	SetExpr::SetExpr(ExprPtr expr, const Token& identifier, const Token& operation, ExprPtr value,
		OpType type) : holder(std::move(expr)), identifier(identifier), operation(operation),
		value(std::move(value)), type(type)
	{
		this->exprType = ExprType::Set;
		set_pos(this->holder->pos_start, this->value->pos_end);
	}

	SetExpr::SetExpr(ExprPtr expr, ExprPtr index, const Token& operation, ExprPtr value, OpType type) : holder(std::move(expr)), index(std::move(index)), operation(operation), value(std::move(value)), type(type)
	{
		this->exprType = ExprType::Set;
		set_pos(this->holder->pos_start, this->value->pos_end);
	}

	Object SetExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string SetExpr::to_string() const
	{
		std::string form;
		switch (type)
		{
		case OpType::DOT:
			form = "Set: %s.%s %s %s";
			break;

		case OpType::BRACKET:
			form = "Set: %s[%s] %s %s";
			break;

		default:
			break;
		}

		return format(form, holder->to_string().c_str(), identifier.lexeme.c_str(), operation.lexeme.c_str(),
			value->to_string().c_str());
	}

	ThisExpr::ThisExpr(const Token& keyword) : keyword(keyword), depth(-1)
	{
		this->exprType = ExprType::This;
		set_pos(this->keyword.pos_start, this->keyword.pos_end);
	}

	Object ThisExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string ThisExpr::to_string() const
	{
		return "This";
	}

	void ThisExpr::resolve(int depth)
	{
		this->depth = depth;
	}

	SuperExpr::SuperExpr(const Token& keyword, const Token& identifier) : keyword(keyword), identifier(identifier), depth(-1)
	{
		this->exprType = ExprType::Super;
		set_pos(this->keyword.pos_start, this->identifier.pos_end);
	}

	Object SuperExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string SuperExpr::to_string() const
	{
		return format("%s.%s", keyword.to_string().c_str(), identifier.to_string().c_str());
	}

	void SuperExpr::resolve(int depth)
	{
		this->depth = depth;
	}

	ListExpr::ListExpr(const Token& left, std::vector<ExprPtr> items, const Token& right) : leftBracket(left),
		items(std::move(items)),
		rightBracket(right)
	{
		this->exprType = ExprType::List;
		set_pos(this->leftBracket.pos_start, this->rightBracket.pos_end);
	}

	Object ListExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string ListExpr::to_string() const
	{
		std::string result = "[";

		size_t length = items.size();
		for (size_t i = 0; i < length; i++)
		{
			result += items[i]->to_string();
			if (i != length - 1)
				result += ", ";
			else
				result.push_back(']');
		}

		return result;
	}

	PackExpr::PackExpr(std::vector<ExprPtr> exprs) : expressions(std::move(exprs))
	{
		this->exprType = ExprType::Pack;
		set_pos(this->expressions.front()->pos_start, this->expressions.back()->pos_end);
	}

	Object PackExpr::accept(ExprVisitor& visitor)
	{
		return visitor.visit(this);
	}

	std::string PackExpr::to_string() const
	{
		std::string result;

		for (auto const& expr : expressions)
		{
			result += expr->to_string();
			result.push_back(',');
		}

		result.pop_back();

		return result;
	}

}