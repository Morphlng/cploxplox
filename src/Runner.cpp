#include "Runner.h"
#include "Common/utils.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "Interpreter/Interpreter.h"
#include <iostream>
#include <vector>

namespace CXX {

	Interpreter Runner::interpreter = Interpreter();
	bool Runner::DEBUG = false;
	Position* Runner::pos_start = nullptr;
	Position* Runner::pos_end = nullptr;

	int Runner::runScript(const std::string& filename)
	{
		interpreter.replEcho = false;
		std::optional<std::string> content = readfile(filename);
		if (content)
			return runCode(filename, *content);

		return -1;
	}

	int Runner::runRepl()
	{
		interpreter.replEcho = true;
		std::string input, text;
		while (true)
		{
			std::cout << "lox > ";
			std::getline(std::cin, input);
			if (input == "exit")
				return 0;

			text = input;
			while (!input.empty() && isIn({ ';', '{', '}' }, text.back()))
			{
				std::cout << "...   ";
				std::getline(std::cin, input);
				text += "\n" + input;
			}

			runCode("<stdio>", text);
		}
	}

	int Runner::runCode(const std::string& filename, const std::string& text)
	{
		Lexer lexer(filename, text);
		std::vector<Token> tokens;
		try
		{
			tokens = lexer.tokenize();
			if (DEBUG)
			{
				for (auto& tok : tokens)
				{
					std::cout << tok.to_string() << "\n";
				}
			}
		}
		catch (const Error& e)
		{
			ErrorReporter::report(e);
			return -ErrorReporter::count();
		}

		Parser parser(tokens);
		std::vector<StmtPtr> ast = parser.parse();
		if (int errCnt = ErrorReporter::count())
		{
			return -errCnt;
		}
		else if (DEBUG)
		{
			for (auto& node : ast)
			{
				std::cout << node->to_string() << "\n";
			}
		}

		Resolver resolver;
		resolver.resolve(ast);
		if (int errCnt = ErrorReporter::count())
		{
			return -errCnt;
		}

		try
		{
			interpreter.interpret(ast);
		}
		catch (const Error& e)
		{
			ErrorReporter::report(e);
			return -ErrorReporter::count();
		}

		return 0;
	}

}