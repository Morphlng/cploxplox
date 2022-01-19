#pragma once
#include <memory>

namespace CXX {

	class Expr;
	class Stmt;
	class Context;

	using ExprPtr = std::shared_ptr<Expr>;
	using StmtPtr = std::shared_ptr<Stmt>;
	using ContextPtr = std::shared_ptr<Context>;

}