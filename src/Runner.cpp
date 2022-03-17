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

#ifdef USE_STRING_VIEW
	static std::vector<std::string> TEXT;
#endif

	int Runner::runScript(const std::string& filename)
	{
		std::optional<std::string> content = readfile(filename);
		if (content) {
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
			while (!input.empty() && isIn({ ';', '{', '}' }, text.back()))
			{
				std::cout << "...   ";
				std::getline(std::cin, input);
				text += "\n" + input;
			}

#ifdef USE_STRING_VIEW
			if (text.length() > 15) {
				TEXT.push_back(std::move(text));
				runCode("<stdio>", TEXT.back(), true);
			}
			else {
				runCode("<stdio>", text, true);
			}
#else
			runCode("<stdio>", text, true);
#endif
		}
	}

	int Runner::runCode(const std::string& filename, const std::string& text, bool repl)
	{
		interpreter.replEcho = repl ? true : false;

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
			lexer.reset();
		}
		catch (const std::exception& e)
		{
			ErrorReporter::report(e);
			return -ErrorReporter::count();
		}

		Parser parser(std::move(tokens));
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
		parser.reset();

		Resolver resolver;
		resolver.resolve(ast);
		if (int errCnt = ErrorReporter::count())
		{
			return -errCnt;
		}

		try
		{
			interpreter.interpret(std::move(ast));
		}
		catch (const std::exception& e)
		{
			ErrorReporter::report(e);
			return -ErrorReporter::count();
		}

		return 0;
	}

}