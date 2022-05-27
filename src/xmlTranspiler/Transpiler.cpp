#include "Common/Error.h"
#include "xmlTranspiler/Transpiler.h"
#include "xmlTranspiler/uuid.h"

namespace CXX
{

	std::string &Transpiler::transpile(const std::vector<StmtPtr> &statements)
	{
		// heading
		xmlCode = "<xml xmlns=\"https://developers.google.com/blockly/xml\">";
		// The length is always 55 (ignore the '\0')

		// transpile statements in global scope
		newScope(statements);

		// insert global variable definition
		if (!variableDB.empty())
		{
			std::string variables = "<variables>";
			for (auto const &[identifier, id] : variableDB)
			{
				variables += "<variable id=\"" + id + "\">" + identifier + "</variable>";
			}
			variables += "</variables>";
			xmlCode.insert(55, variables);
		}

		// ending
		xmlCode += "</xml>";

		return xmlCode;
	}

	void Transpiler::visit(const ExpressionStmt *expressionStmt)
	{
		translate(expressionStmt->expr.get());
	}

	void Transpiler::visit(const VarDeclarationStmt *varStmt)
	{
		// Global Variable Define (Blockly Style)
		std::string var_id = sole::uuid4().base62();
		variableDB.emplace(varStmt->identifier.lexeme, var_id);

		if (varStmt->expr)
		{
			xmlCode += "<block type=\"variables_set\" id=\"" + sole::uuid4().base62() + "\">";
			xmlCode += "<field name=\"VAR\" id=\"" + var_id + "\">" + varStmt->identifier.lexeme + "</field>";
			xmlCode += "<value name=\"VALUE\">";
			translate(varStmt->expr.value().get());
			xmlCode += "</value>";
			xmlCode += "</block>";
		}
	}

	void Transpiler::visit(std::shared_ptr<FuncDeclarationStmt> funcDeclStmt)
	{
		std::string blockType = "procedures_defnoreturn";
		const ReturnStmt *retPtr{nullptr};

		for (auto it = funcDeclStmt->body.rbegin(); it != funcDeclStmt->body.rend(); it++)
		{
			if ((*it)->stmtType == StmtType::Return)
			{
				auto rs = static_cast<ReturnStmt *>(it->get());
				if (!rs->expr)
					continue;
				else
				{
					blockType = "procedures_defreturn";
					retPtr = rs;
					break;
				}
			}
		}

		Function func{
			funcDeclStmt->name.lexeme, // name
			{},						   // arg_names
			retPtr ? true : false	   // has return
		};

		// Define block
		xmlCode += "<block type=\"" + blockType + "\" id=\"" + sole::uuid4().base62() + "\">";

		// Register parameters
		if (!funcDeclStmt->params.empty())
		{
			std::transform(funcDeclStmt->params.begin(), funcDeclStmt->params.end(), std::back_inserter(func.arg_names), [](const Token &tok)
						   { return tok.lexeme; });

			xmlCode += "<mutation>";
			for (auto const &arg : func.arg_names)
			{
				std::string var_id = sole::uuid4().base62();
				variableDB.emplace(arg, var_id);
				xmlCode += "<arg name=\"" + arg + "\" varid=\"" + var_id + "\"></arg>";
			}
			xmlCode += "</mutation>";
		}
		// Function name
		xmlCode += "<field name=\"NAME\">" + func.name + "</field>";

		// Register Function (so you can do recursive call);
		functionDB.emplace(func.name, func);

		// Body
		xmlCode += "<statement name=\"STACK\">";
		newScope(funcDeclStmt->body);
		xmlCode += "</statement>";

		// Return stmt need to be parsed alone
		if (retPtr)
		{
			visitRet(retPtr);
		}

		xmlCode += "</block>";
	}

	void Transpiler::visit(const ClassDeclarationStmt *classDeclStmt)
	{
		xmlCode += "<comment pinned=\"true\">TODO:ClassDeclarationStmt</comment>";
	}

	void Transpiler::visit(const BlockStmt *blockStmt)
	{
		newScope(blockStmt->statements);
	}

	void Transpiler::visit(const IfStmt *ifStmt)
	{
		if (ifStmt->thenBranch->stmtType == StmtType::Return)
		{
			xmlCode += "<block type=\"procedures_ifreturn\" id=\"" + sole::uuid4().base62() + "\">";
			xmlCode += "<mutation value=\"1\"></mutation>";
			xmlCode += "<value name=\"CONDITION\">";
			translate(ifStmt->condition.get());
			xmlCode += "</value>";
			visitRet(static_cast<ReturnStmt *>(ifStmt->thenBranch.get()), "VALUE");
			xmlCode += "</block>";
		}
		else
		{
			xmlCode += "<block type=\"controls_if\" id=\"" + sole::uuid4().base62() + "\">";
			if (ifStmt->elseBranch)
				xmlCode += "<mutation else=\"1\"></mutation>";

			// if-condition
			xmlCode += "<value name=\"IF0\">";
			translate(ifStmt->condition.get());
			xmlCode += "</value>";

			// then
			xmlCode += "<statement name=\"DO0\">";
			translate(ifStmt->thenBranch.get());
			xmlCode += "</statement>";

			// else
			if (ifStmt->elseBranch)
			{
				xmlCode += "<statement name=\"ELSE\">";
				translate(ifStmt->elseBranch.value().get());
				xmlCode += "</statement>";
			}

			xmlCode += "</block>";
		}
	}

	void Transpiler::visit(const WhileStmt *whileStmt)
	{
		xmlCode += "<block type=\"controls_whileUntil\" id=\"" + sole::uuid4().base62() + "\">";
		xmlCode += "<field name=\"MODE\">WHILE</field>";
		// condition
		xmlCode += "<value name=\"BOOL\">";
		translate(whileStmt->condition.get());
		xmlCode += "</value>";
		// then
		xmlCode += "<statement name=\"DO\">";
		translate(whileStmt->body.get());
		xmlCode += "</statement>";
		xmlCode += "</block>";
	}

	void Transpiler::visit(const ForStmt *forStmt)
	{
		xmlCode += "<block type=\"controls_for\" id=\"" + sole::uuid4().base62() + "\">";

		std::string from;
		if (forStmt->initializer)
		{
			Stmt *init = forStmt->initializer->get();
			if (init->stmtType == StmtType::VarDecl)
			{
				VarDeclarationStmt *varStmt = static_cast<VarDeclarationStmt *>(init);

				std::string &var_id = variableDB[varStmt->identifier.lexeme];
				if (var_id.empty())
					var_id = sole::uuid4().base62();

				xmlCode += "<field name=\"VAR\" id=\"" + var_id + "\">" + varStmt->identifier.lexeme + "</field>";
				if (varStmt->expr && varStmt->expr.value()->exprType == ExprType::Literal)
				{
					LiteralExpr *fromExpr = static_cast<LiteralExpr *>(varStmt->expr->get());
					from = "<block type=\"math_number\" id=\"" + sole::uuid4().base62() + "\">"
																						  "<field name=\"NUM\">" +
						   fromExpr->value.to_string() + "</field></block>";
				}
			}
			else if (init->stmtType == StmtType::Expression && static_cast<ExpressionStmt *>(init)->expr->exprType == ExprType::Variable)
			{
				VariableExpr *varExpr = static_cast<VariableExpr *>(static_cast<ExpressionStmt *>(init)->expr.get());
				std::string &var_id = variableDB[varExpr->identifier.lexeme];
				if (var_id.empty())
					var_id = sole::uuid4().base62();

				xmlCode += "<field name=\"VAR\" id=\"" + var_id + "\">" + varExpr->identifier.lexeme + "</field>";
				from = "<block type=\"variables_get\" id=\"" + sole::uuid4().base62() + "\">"
																						"<field name=\"VAR\"> id=\"" +
					   var_id + "\">" + varExpr->identifier.lexeme + "</field></block>";
			}
		}
		else
		{
			std::string &var_id = variableDB["i"];
			if (var_id.empty())
				var_id = sole::uuid4().base62();
			xmlCode += "<field name=\"VAR\" id=\"" + var_id + "\">i</field>";
		}

		xmlCode += "<value name=\"FROM\">"
				   "<shadow type=\"math_number\" id=\"" +
				   sole::uuid4().base62() + "\">"
											"<field name=\"NUM\">1</field>"
											"</shadow>";
		xmlCode += from;
		xmlCode += "</value>";

		xmlCode += "<value name=\"TO\">"
				   "<shadow type=\"math_number\" id=\"" +
				   sole::uuid4().base62() + "\">"
											"<field name=\"NUM\">10</field>"
											"</shadow>";

		if (forStmt->condition && forStmt->condition.value()->exprType == ExprType::Binary)
		{
			BinaryExpr *binExpr = static_cast<BinaryExpr *>(forStmt->condition->get());
			if (binExpr->right->exprType == ExprType::Literal)
			{
				translate(binExpr->right.get());
			}
		}
		xmlCode += "</value>";

		xmlCode += "<value name=\"BY\">"
				   "<shadow type=\"math_number\" id=\"" +
				   sole::uuid4().base62() + "\">"
											"<field name=\"NUM\">1</field>"
											"</shadow>";

		if (forStmt->increment && forStmt->increment.value()->exprType == ExprType::Assignment)
		{
			// +=
			AssignmentExpr *assignExpr = static_cast<AssignmentExpr *>(forStmt->increment->get());
			translate(assignExpr->value.get());
		}
		xmlCode += "</value>";

		xmlCode += "<statement name=\"DO\">";
		translate(forStmt->body.get());
		xmlCode += "</statement>";

		xmlCode += "</block>";
	}

	void Transpiler::visit(const BreakStmt *breakStmt)
	{
		xmlCode += "<block type=\"controls_flow_statements\" id=\"" + sole::uuid4().base62() + "\">";
		xmlCode += "<field name=\"FLOW\">BREAK</field>";
		xmlCode += "</block>";
	}

	void Transpiler::visit(const ContinueStmt *continueStmt)
	{
		xmlCode += "<block type=\"controls_flow_statements\" id=\"" + sole::uuid4().base62() + "\">";
		xmlCode += "<field name=\"FLOW\">CONTINUE</field>";
		xmlCode += "</block>";
	}

	void Transpiler::visit(const ReturnStmt *returnStmt)
	{
		// do nothing
		// ReturnStmt will use visitRet() function
	}

	void Transpiler::visit(const ImportStmt *importStmt)
	{
		xmlCode += "<comment pinned=\"true\">TODO:ImportStmt</comment>";
	}

	void Transpiler::visit(const PackStmt *packStmt)
	{
		for (auto &stmt : packStmt->statements)
			translate(stmt.get());
	}

	Object Transpiler::visit(const BinaryExpr *binaryExpr)
	{
		std::string block_type = (binaryExpr->op.type < TokenType::BANGEQ) ? "math_arithmetic" : "logic_compare";

		// Define Block
		xmlCode += "<block type=\"" + block_type + "\" id=\"" + sole::uuid4().base62() + "\">";

		// Operator
		xmlCode += "<field name=\"OP\">";
		switch (binaryExpr->op.type)
		{
		case TokenType::PLUS:
			xmlCode += "ADD";
			break;

		case TokenType::MINUS:
			xmlCode += "MINUS";
			break;

		case TokenType::MUL:
			xmlCode += "MULTIPLY";
			break;

		case TokenType::DIV:
			xmlCode += "DIVIDE";
			break;

		case TokenType::GT:
			xmlCode += "GT";
			break;

		case TokenType::GTE:
			xmlCode += "GTE";
			break;

		case TokenType::LT:
			xmlCode += "LT";
			break;

		case TokenType::LTE:
			xmlCode += "LTE";
			break;

		case TokenType::EQEQ:
			xmlCode += "EQ";
			break;

		case TokenType::BANGEQ:
			xmlCode += "NEQ";
			break;

		default:
			ErrorReporter::report(std::runtime_error("Unsupported binary operation"));
			return {};
		}
		xmlCode += "</field>";

		// Left operand
		xmlCode += "<value name=\"A\">";
		// default value(1)
		xmlCode += "<shadow type=\"math_number\" id=\"" + sole::uuid4().base62() + "\">";
		xmlCode += "<field name=\"NUM\">1</field>";
		xmlCode += "</shadow>";
		// transpile left operand
		translate(binaryExpr->left.get());
		xmlCode += "</value>";

		// Right operand
		xmlCode += "<value name=\"B\">";
		// default value(1)
		xmlCode += "<shadow type=\"math_number\" id=\"" + sole::uuid4().base62() + "\">";
		xmlCode += "<field name=\"NUM\">1</field>";
		xmlCode += "</shadow>";
		// transpile right operand
		translate(binaryExpr->right.get());
		xmlCode += "</value>";

		// End of block
		xmlCode += "</block>";

		return {};
	}

	Object Transpiler::visit(const UnaryExpr *unaryExpr)
	{
		if (unaryExpr->op.type == TokenType::BANG)
		{
			xmlCode += "<block type=\"logic_negate\" id=\"" + sole::uuid4().base62() + "\">";
			xmlCode += "<value name=\"BOOL\">";
			translate(unaryExpr->expr.get());
			xmlCode += "</value>";
			xmlCode += "</block>";
		}
		else
			xmlCode += "<comment pinned=\"true\">TODO:UnaryExpr(-)</comment>";

		return {};
	}

	Object Transpiler::visit(const LiteralExpr *literalExpr)
	{
		std::string blockType("logic_null");
		std::string value = literalExpr->value.to_string();
		std::string field_name;

		switch (literalExpr->value.type)
		{
		case ObjectType::NUMBER:
			blockType = "math_number";
			field_name = "NUM";
			break;

		case ObjectType::STRING:
			blockType = "text";
			field_name = "TEXT";
			break;

		case ObjectType::BOOL:
			blockType = "logic_boolean";
			field_name = "BOOL";
			std::for_each(value.begin(), value.end(), [](char &c)
						  { c = std::toupper(c); });
			break;

		default:
			break;
		}

		xmlCode += "<block type=\"" + blockType + "\" id=\"" + sole::uuid4().base62() + "\">";
		if (!field_name.empty())
		{
			xmlCode += "<field name=\"" + field_name + "\">" + value + "</field>";
		}

		xmlCode += "</block>";
		return {};
	}

	Object Transpiler::visit(const VariableExpr *variableExpr)
	{
		std::string var_id = varID(variableExpr);

		xmlCode += "<block type=\"variables_get\" id=\"" + sole::uuid4().base62() + "\">";
		xmlCode += "<field name=\"VAR\" id=\"" + var_id + "\">" + variableExpr->identifier.lexeme + "</field>";
		xmlCode += "</block>";

		return {};
	}

	Object Transpiler::visit(const AssignmentExpr *assignmentExpr)
	{
		std::string var_id = varID(assignmentExpr->identifier.lexeme);

		xmlCode += "<block type=\"variables_set\" id=\"" + sole::uuid4().base62() + "\">";
		xmlCode += "<field name=\"VAR\" id=\"" + var_id + "\">" + assignmentExpr->identifier.lexeme + "</field>";
		xmlCode += "<value name=\"VALUE\">";
		if (assignmentExpr->operation.type == TokenType::EQ)
		{
			translate(assignmentExpr->value.get());
		}
		else
		{
			xmlCode += "<block type=\"math_arithmetic\" id=\"" + sole::uuid4().base62() + "\">";

			// operation
			xmlCode += "<field name=\"OP\">";
			switch (assignmentExpr->operation.type)
			{
			case TokenType::PLUS_EQUAL:
				xmlCode += "ADD";
				break;
			case TokenType::MINUS_EQUAL:
				xmlCode += "MINUS";
				break;
			case TokenType::MUL_EQUAL:
				xmlCode += "MULTIPLY";
				break;
			case TokenType::DIV_EQUAL:
				xmlCode += "DIVIDE";
				break;
			default:
				break;
			}
			xmlCode += "</field>";

			// origin
			xmlCode += "<value name=\"A\">";
			xmlCode += "<block type=\"variables_get\" id=\"" + sole::uuid4().base62() + "\">";
			xmlCode += "<field name=\"VAR\" id=\"" + var_id + "\">" + assignmentExpr->identifier.lexeme + "</field>";
			xmlCode += "</block>";
			xmlCode += "</value>";

			// delta value
			xmlCode += "<value name=\"B\">";
			translate(assignmentExpr->value.get());
			xmlCode += "</value>";

			xmlCode += "</block>";
		}
		xmlCode += "</value>";
		xmlCode += "</block>";
		return {};
	}

	Object Transpiler::visit(const TernaryExpr *ternaryExpr)
	{
		xmlCode += "<block type=\"logic_ternary\" id=\"" + sole::uuid4().base62() + "\">";
		// if
		xmlCode += "<value name=\"IF\">";
		translate(ternaryExpr->expr.get());
		xmlCode += "</value>";
		// then
		xmlCode += "<value name=\"THEN\">";
		translate(ternaryExpr->thenBranch.get());
		xmlCode += "</value>";
		// else
		xmlCode += "<value name=\"ELSE\">";
		translate(ternaryExpr->elseBranch.get());
		xmlCode += "</value>";

		xmlCode += "</block>";
		return {};
	}

	Object Transpiler::visit(std::shared_ptr<LambdaExpr> lambdaExpr)
	{
		xmlCode += "<comment pinned=\"true\">TODO:LambdaExpr</comment>";
		return {};
	}

	Object Transpiler::visit(const OrExpr *orExpr)
	{
		xmlCode += "<block type=\"logic_operation\" id=\"" + sole::uuid4().base62() + "\">";

		xmlCode += "<field name=\"OP\">OR</field>";
		// Left operand
		xmlCode += "<value name=\"A\">";
		// default value(false)
		xmlCode += "<shadow type=\"logic_boolean\" id=\"" + sole::uuid4().base62() + "\">";
		xmlCode += "<field name=\"BOOL\">FALSE</field>";
		xmlCode += "</shadow>";
		// TODO: if left or right are not boolean, blockly doesn't accept it.
		translate(orExpr->left.get());
		xmlCode += "</value>";
		// Right Operand;
		xmlCode += "<value name=\"B\">";
		// default value(false)
		xmlCode += "<shadow type=\"logic_boolean\" id=\"" + sole::uuid4().base62() + "\">";
		xmlCode += "<field name=\"BOOL\">FALSE</field>";
		xmlCode += "</shadow>";
		translate(orExpr->right.get());
		xmlCode += "</value>";

		xmlCode += "</block>";
		return {};
	}

	Object Transpiler::visit(const AndExpr *andExpr)
	{
		xmlCode += "<block type=\"logic_operation\" id=\"" + sole::uuid4().base62() + "\">";

		xmlCode += "<field name=\"OP\">AND</field>";
		// Left operand
		xmlCode += "<value name=\"A\">";
		// default value(false)
		xmlCode += "<shadow type=\"logic_boolean\" id=\"" + sole::uuid4().base62() + "\">";
		xmlCode += "<field name=\"BOOL\">FALSE</field>";
		xmlCode += "</shadow>";
		// TODO: if left or right are not boolean, blockly doesn't accept it.
		translate(andExpr->left.get());
		xmlCode += "</value>";
		// Right Operand;
		xmlCode += "<value name=\"B\">";
		// default value(false)
		xmlCode += "<shadow type=\"logic_boolean\" id=\"" + sole::uuid4().base62() + "\">";
		xmlCode += "<field name=\"BOOL\">FALSE</field>";
		xmlCode += "</shadow>";
		translate(andExpr->right.get());
		xmlCode += "</value>";

		xmlCode += "</block>";
		return {};
	}

	Object Transpiler::visit(const IncrementExpr *incrementExpr)
	{
		if (incrementExpr->holder->exprType == ExprType::Variable)
		{
			auto pvar = static_cast<VariableExpr *>(incrementExpr->holder.get());
			std::string var_id = varID(pvar);

			xmlCode += "<block type=\"math_change\" id=\"" + sole::uuid4().base62() + "\">";
			xmlCode += "<field name=\"VAR\" id=\"" + var_id + "\">" + pvar->identifier.lexeme + "</field>";
			xmlCode += "<value name=\"DELTA\">";
			xmlCode += "<shadow type=\"math_number\" id=\"" + sole::uuid4().base62() + "\">";
			xmlCode += "<field name=\"NUM\">1</field>";
			xmlCode += "</shadow>";
			xmlCode += "</value>";
			xmlCode += "</block>";
		}
		else
			xmlCode += "<comment pinned=\"true\">TODO:IncrementExpr(Retrieve)</comment>";

		return {};
	}

	Object Transpiler::visit(const DecrementExpr *decrementExpr)
	{
		if (decrementExpr->holder->exprType == ExprType::Variable)
		{
			auto pvar = static_cast<VariableExpr *>(decrementExpr->holder.get());
			std::string var_id = varID(pvar);

			xmlCode += "<block type=\"math_change\" id=\"" + sole::uuid4().base62() + "\">";
			xmlCode += "<field name=\"VAR\" id=\"" + var_id + "\">" + pvar->identifier.lexeme + "</field>";
			xmlCode += "<value name=\"DELTA\">";
			xmlCode += "<shadow type=\"math_number\" id=\"" + sole::uuid4().base62() + "\">";
			xmlCode += "<field name=\"NUM\">-1</field>";
			xmlCode += "</shadow>";
			xmlCode += "</value>";
			xmlCode += "</block>";
		}
		else
			xmlCode += "<comment pinned=\"true\">TODO:DecrementExpr(Retrieve)</comment>";

		return {};
	}

	Object Transpiler::visit(const CallExpr *callExpr)
	{
		// func fib(n){if(n<2)return n;return fib(n-1)+fib(n-2);}
		if (callExpr->callee->exprType == ExprType::Variable)
		{
			auto varExpr = static_cast<VariableExpr *>(callExpr->callee.get());
			auto it = functionDB.find(varExpr->identifier.lexeme);
			if (it == functionDB.end())
			{
				std::string msg = "Calling undefined function \"" + varExpr->identifier.lexeme + "\"in CallExpr";
				ErrorReporter::report(std::runtime_error(msg));
				xmlCode += "<comment pinned=\"true\">Error:" + msg + "</comment>";
				return {};
			}

			Function &func = it->second;
			std::string blockType = func.hasRet ? "procedures_callreturn" : "procedures_callnoreturn";
			// Define block
			xmlCode += "<block type=\"" + blockType + "\" id=\"" + sole::uuid4().base62() + "\">";
			// Mutation(parameter)
			xmlCode += "<mutation name=\"" + func.name + "\">";
			for (auto &arg : func.arg_names)
			{
				xmlCode += "<arg name=\"" + arg + "\"></arg>";
			}
			xmlCode += "</mutation>";
			// translate arguments
			for (size_t i = 0; i < callExpr->arguments.size(); i++)
			{
				xmlCode += "<value name=\"ARG" + std::to_string(i) + "\">";
				translate(callExpr->arguments[i].get());
				xmlCode += "</value>";
			}

			xmlCode += "</block>";
		}
		else
			xmlCode += "<comment pinned=\"true\">TODO:CallExpr only support Function call for now</comment>";

		return {};
	}

	Object Transpiler::visit(const RetrieveExpr *retrieveExpr)
	{
		xmlCode += "<comment pinned=\"true\">TODO:RetrieveExpr</comment>";
		return {};
	}

	Object Transpiler::visit(const SetExpr *setExpr)
	{
		xmlCode += "<comment pinned=\"true\">TODO:SetExpr</comment>";
		return {};
	}

	Object Transpiler::visit(const ThisExpr *thisExpr)
	{
		xmlCode += "<comment pinned=\"true\">TODO:ThisExpr</comment>";
		return {};
	}

	Object Transpiler::visit(const SuperExpr *superExpr)
	{
		xmlCode += "<comment pinned=\"true\">TODO:SuperExpr</comment>";
		return {};
	}

	Object Transpiler::visit(const ListExpr *listExpr)
	{
		xmlCode += "<block type=\"lists_create_with\" id=\"" + sole::uuid4().base62() + "\">";

		if (listExpr->items.empty())
		{
			xmlCode += "<mutation items=\"0\"></mutation>";
		}
		else
		{
			xmlCode += "<mutation items=\"" + std::to_string(listExpr->items.size()) + "\"></mutation>";
			for (size_t i = 0; i < listExpr->items.size(); i++)
			{
				xmlCode += "<value name=\"ADD" + std::to_string(i) + "\">";
				translate(listExpr->items[i].get());
				xmlCode += "</value>";
			}
		}

		xmlCode += "</block>";

		return {};
	}

	Object Transpiler::visit(const PackExpr *packExpr)
	{
		for (auto &expr : packExpr->expressions)
			translate(expr.get());

		return {};
	}

	void Transpiler::translate(Stmt *stmt)
	{
		stmt->accept(*this);
	}

	void Transpiler::translate(Expr *expr)
	{
		expr->accept(*this);
	}

	void Transpiler::visitRet(const ReturnStmt *returnStmt, const std::string &value_name)
	{
		if (returnStmt->expr)
		{
			xmlCode += "<value name=\"" + value_name + "\">";
			translate(returnStmt->expr.value().get());
			xmlCode += "</value>";
		}
	}

	void Transpiler::newScope(const std::vector<StmtPtr> &statements)
	{
		if (size_t size = statements.size())
		{

			for (size_t i = 0; i < size - 1; i++)
			{
				translate(statements[i].get());
				// replace </block> with <next>
				xmlCode.replace(xmlCode.length() - 8, xmlCode.length(), "<next>");
			}

			translate(statements.back().get());

			for (size_t i = 0; i < size - 1; i++)
			{
				xmlCode += "</next>";
				xmlCode += "</block>";
			}
		}
	}

	std::string Transpiler::varID(const VariableExpr *variableExpr)
	{
		auto it = variableDB.find(variableExpr->identifier.lexeme);
		if (it == variableDB.end())
		{
			ErrorReporter::report(std::runtime_error("Using undefined variable in VariableExpr"));
			return "0000-0000-0000-0000";
		}

		return it->second;
	}

	std::string Transpiler::varID(const std::string &varName)
	{
		auto it = variableDB.find(varName);
		if (it == variableDB.end())
		{
			ErrorReporter::report(std::runtime_error("Using undefined variable in VariableExpr"));
			return "0000-0000-0000-0000";
		}

		return it->second;
	}
}