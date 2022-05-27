#include "Runner.h"
#include "Common/utils.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "Interpreter/Interpreter.h"
#include "xmlTranspiler/Transpiler.h"
#include <iostream>
#include <vector>

namespace CXX
{

	Interpreter Runner::interpreter = Interpreter();
	Transpiler Runner::transpiler = Transpiler();
	bool Runner::DEBUG = false;
	Position *Runner::pos_start = nullptr;
	Position *Runner::pos_end = nullptr;

#ifdef USE_STRING_VIEW
	static std::vector<std::string> TEXT;
#endif

	int Runner::runScript(const std::string &filename)
	{
		std::optional<std::string> content = readfile(filename);
		if (content)
		{
#ifdef USE_STRING_VIEW
			TEXT.push_back(std::move(content.value()));
			return runCode(filename, TEXT.back());
#else
			return runCode(filename, *content);
#endif
		}

		return -1;
	}

	int Runner::runRepl()
	{
		std::string input, text;
		while (true)
		{
			std::cout << "lox > ";
			std::getline(std::cin, input);
			if (input == "exit")
				return 0;

			text = input;
			while (!input.empty() && isIn({';', '{', '}'}, text.back()))
			{
				std::cout << "...   ";
				std::getline(std::cin, input);
				text += "\n" + input;
			}

#ifdef USE_STRING_VIEW
			if (text.length() > 15)
			{
				TEXT.push_back(std::move(text));
				runCode("<stdio>", TEXT.back(), true);
			}
			else
#endif
				runCode("<stdio>", text, true);
		}
	}

	std::optional<std::vector<StmtPtr>> getAST(const std::string &filename, const std::string &text)
	{
		Lexer lexer(filename, text);
		std::vector<Token> tokens;
		try
		{
			tokens = std::move(lexer.tokenize());
			if (Runner::DEBUG)
			{
				for (auto &tok : tokens)
				{
					std::cout << tok.to_string() << "\n";
				}
			}
		}
		catch (const std::exception &e)
		{
			ErrorReporter::report(e);
			return std::nullopt;
		}

		Parser parser(std::move(tokens));
		std::vector<StmtPtr> ast = parser.parse();
		if (int errCnt = ErrorReporter::count())
		{
			return std::nullopt;
		}
		else if (Runner::DEBUG)
		{
			for (auto &node : ast)
			{
				std::cout << node->to_string() << "\n";
			}
		}

		Resolver resolver;
		resolver.resolve(ast);
		if (int errCnt = ErrorReporter::count())
		{
			return std::nullopt;
		}

		return ast;
	}

	int Runner::runTranspile()
	{
		std::string input, text;
		while (true)
		{
			std::cout << "lox > ";
			std::getline(std::cin, input);
			if (input == "exit")
				return 0;

			text = input;
			while (!input.empty() && isIn({';', '{', '}'}, text.back()))
			{
				std::cout << "...   ";
				std::getline(std::cin, input);
				text += "\n" + input;
			}

			auto ast_ptr = getAST("<stdio>", text);
			if (!ast_ptr)
				continue;

			std::vector<StmtPtr> &ast = ast_ptr.value();
			auto &xmlCode = Runner::transpiler.transpile(ast);
			std::cout << xmlCode << "\n";

			ErrorReporter::reset();
		}

		return 0;
	}

	int Runner::runCode(const std::string &filename, const std::string &text, bool repl)
	{
		interpreter.replEcho = repl ? true : false;

		auto ast_ptr = getAST(filename, text);
		if (!ast_ptr)
			return -1;

		std::vector<StmtPtr> &ast = ast_ptr.value();

		try
		{
			interpreter.interpret(std::move(ast));
		}
		catch (const std::exception &e)
		{
			ErrorReporter::report(e);
			return -ErrorReporter::count();
		}

		return 0;
	}

}