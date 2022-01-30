#include "Parser/Stmt.h"
#include "Parser/Expr.h"
#include "Common/utils.h"

namespace CXX {

	ExpressionStmt::ExpressionStmt(ExprPtr expr) : expr(std::move(expr))
	{
		this->stmtType = StmtType::Expression;
		set_pos(this->expr->pos_start, this->expr->pos_end);
	}

	void ExpressionStmt::accept(StmtVisitor& visitor)
	{
		visitor.visit(this);
	}

	std::string ExpressionStmt::to_string() const
	{
		return expr->to_string();
	}

	VarDeclarationStmt::VarDeclarationStmt(const Token& identifier, std::optional<ExprPtr> expr) : identifier(identifier),
		expr(std::move(expr))
	{
		this->stmtType = StmtType::VarDecl;
		if (this->expr)
		{
			set_pos(this->identifier.pos_start, this->expr.value()->pos_end);
		}
		else
		{
			set_pos(this->identifier.pos_start, this->identifier.pos_end);
		}
	}

	void VarDeclarationStmt::accept(StmtVisitor& visitor)
	{
		visitor.visit(this);
	}

	std::string VarDeclarationStmt::to_string() const
	{
		if (expr)
			return format("VAR %s = %s", identifier.lexeme.c_str(), expr.value()->to_string().c_str());

		return format("VAR %s", identifier.lexeme.c_str());
	}

	FuncDeclarationStmt::FuncDeclarationStmt(const Token& name, std::vector<Token> params, std::vector<ExprPtr> default_values, std::vector<StmtPtr> body) :
		name(name), params(std::move(params)), default_values(std::move(default_values)), body(std::move(body))
	{
		this->stmtType = StmtType::FuncDecl;
		if (this->body.empty())
			set_pos(this->name.pos_start, this->name.pos_end);
		else
			set_pos(this->name.pos_start, this->body.back()->pos_end);
	}

	void FuncDeclarationStmt::accept(StmtVisitor& visitor)
	{
		visitor.visit(shared_from_this());
	}

	std::string FuncDeclarationStmt::to_string() const
	{
		std::string result = format("FUNC %s(", name.lexeme.c_str());
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

	ClassDeclarationStmt::ClassDeclarationStmt(const Token& identifier,
		std::vector<std::shared_ptr<FuncDeclarationStmt>> methods,
		std::optional<std::shared_ptr<VariableExpr>> superclass)
		: name(identifier), methods(std::move(methods)), superClass(std::move(superclass))
	{
		this->stmtType = StmtType::ClassDecl;
		this->pos_start = this->name.pos_start;
		this->pos_end = this->methods.empty() ? (this->superClass.has_value() ? this->superClass.value()->pos_end
			: this->name.pos_end)
			: this->methods.back()->pos_end;
	}

	void ClassDeclarationStmt::accept(StmtVisitor& visitor)
	{
		visitor.visit(this);
	}

	std::string ClassDeclarationStmt::to_string() const
	{
		return format("<CLASS %s>", name.lexeme.c_str());
	}

	BlockStmt::BlockStmt(std::vector<StmtPtr> statements) : statements(std::move(statements))
	{
		this->stmtType = StmtType::Block;
		if (!this->statements.empty())
			set_pos(this->statements.begin()->get()->pos_start, this->statements.rbegin()->get()->pos_end);
		else
			set_pos(Position::preset, Position::preset);
	}

	void BlockStmt::accept(StmtVisitor& visitor)
	{
		visitor.visit(this);
	}

	std::string BlockStmt::to_string() const
	{
		std::string result = "{\n";

		for (auto& stmt : statements)
		{
			result += stmt->to_string();
			result.push_back('\n');
		}
		result.push_back('}');
		return result;
	}

	IfStmt::IfStmt(ExprPtr condition, StmtPtr thenBranch, std::optional<StmtPtr> elseBranch) : condition(
		std::move(condition)),
		thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch))
	{
		this->stmtType = StmtType::If;
		if (this->elseBranch)
		{
			set_pos(this->condition->pos_start, this->elseBranch.value()->pos_end);
		}
		else
		{
			set_pos(this->condition->pos_start, this->thenBranch->pos_end);
		}
	}

	void IfStmt::accept(StmtVisitor& visitor)
	{
		visitor.visit(this);
	}

	std::string IfStmt::to_string() const
	{
		std::string result = "IF " + this->condition->to_string();
		result.push_back('\n');

		result += "THEN " + this->thenBranch->to_string();

		if (this->elseBranch)
		{
			result.push_back('\n');
			result += "ELSE " + elseBranch.value()->to_string();
		}

		return result;
	}

	WhileStmt::WhileStmt(ExprPtr condition, StmtPtr body) : condition(std::move(condition)), body(std::move(body))
	{
		this->stmtType = StmtType::While;
		if (this->body)
		{
			set_pos(this->condition->pos_start, this->body->pos_end);
		}
		else
		{
			set_pos(this->condition->pos_start, this->condition->pos_end);
		}
	}

	void WhileStmt::accept(StmtVisitor& visitor)
	{
		visitor.visit(this);
	}

	std::string WhileStmt::to_string() const
	{
		std::string result = "WHILE " + this->condition->to_string();

		if (this->body)
		{
			result.push_back('\n');
			result += "THEN " + this->body->to_string();
		}

		return result;
	}

	ForStmt::ForStmt(std::optional<StmtPtr> initializer, std::optional<ExprPtr> condition, std::optional<ExprPtr> increment,
		StmtPtr body) : initializer(std::move(initializer)), condition(std::move(condition)),
		increment(std::move(increment)), body(std::move(body))
	{
		this->stmtType = StmtType::For;
		if (this->initializer)
		{
			set_pos(this->initializer.value()->pos_start, this->body->pos_end);
		}
		else if (this->condition)
		{
			set_pos(this->condition.value()->pos_start, this->body->pos_end);
		}
		else if (this->increment)
		{
			set_pos(this->increment.value()->pos_start, this->body->pos_end);
		}
		else
		{
			set_pos(this->body->pos_start, this->body->pos_end);
		}
	}

	void ForStmt::accept(StmtVisitor& visitor)
	{
		visitor.visit(this);
	}

	std::string ForStmt::to_string() const
	{
		std::string result = "for(";
		if (initializer)
		{
			result += initializer.value()->to_string();
		}
		result.push_back(';');

		if (condition)
		{
			result += condition.value()->to_string();
		}
		result.push_back(';');

		if (increment)
		{
			result += increment.value()->to_string();
		}
		result += ")\n" + body->to_string();

		return result;
	}

	BreakStmt::BreakStmt(const Token& keyword) : keyword(keyword)
	{
		this->stmtType = StmtType::Break;
		set_pos(this->keyword.pos_start, this->keyword.pos_end);
	}

	void BreakStmt::accept(StmtVisitor& visitor)
	{
		visitor.visit(this);
	}

	std::string BreakStmt::to_string() const
	{
		return "BREAK;";
	}

	ContinueStmt::ContinueStmt(const Token& keyword) : keyword(keyword)
	{
		this->stmtType = StmtType::Continue;
		set_pos(this->keyword.pos_start, this->keyword.pos_end);
	}

	void ContinueStmt::accept(StmtVisitor& visitor)
	{
		visitor.visit(this);
	}

	std::string ContinueStmt::to_string() const
	{
		return "CONTINUE;";
	}

	ReturnStmt::ReturnStmt(const Token& keyword, std::optional<ExprPtr> expr) : keyword(keyword), expr(std::move(expr))
	{
		this->stmtType = StmtType::Return;
		if (this->expr)
		{
			set_pos(this->keyword.pos_start, this->expr.value()->pos_end);
		}
		else
		{
			set_pos(this->keyword.pos_start, this->keyword.pos_end);
		}
	}

	void ReturnStmt::accept(StmtVisitor& visitor)
	{
		visitor.visit(this);
	}

	std::string ReturnStmt::to_string() const
	{
		if (expr)
			return format("RETURN %s;", expr.value()->to_string().c_str());
		else
			return "RETURN;";
	}

	PackStmt::PackStmt(std::vector<StmtPtr> stmts) : statements(std::move(stmts))
	{
		this->stmtType = StmtType::Pack;
		set_pos(this->statements.front()->pos_start, this->statements.back()->pos_end);
	}

	void PackStmt::accept(StmtVisitor& visitor)
	{
		visitor.visit(this);
	}

	std::string PackStmt::to_string() const
	{
		std::string result;

		for (auto const& stmt : statements) {
			result += stmt->to_string();
			result.push_back('\n');
		}

		return result;
	}

	ImportStmt::ImportStmt(const Token& keyword, std::map<Token, std::optional<Token>> symbols, const Token& filepath)
		:keyword(keyword), symbols(std::move(symbols)), filepath(filepath)
	{
		this->stmtType = StmtType::Import;
		set_pos(keyword.pos_start, filepath.pos_end);
	}

	void ImportStmt::accept(StmtVisitor& visitor)
	{
		visitor.visit(this);
	}

	std::string ImportStmt::to_string() const
	{
		std::string result = "import { ";
		for (auto& [name, alias] : symbols) {
			result += name.lexeme;
			if (alias) {
				result += " as " + alias->lexeme;
			}
			result += ",";
		}
		if (result.back() == ',')
			result.pop_back();

		result += "} from " + filepath.lexeme;

		return result;
	}

}