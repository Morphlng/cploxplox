#pragma once
#include <string>

namespace CXX
{

	class Interpreter;
	class Transpiler;
	class Position;

	class Runner
	{
	public:
		static int runScript(const std::string &filename);

		static int runRepl();

		static int runTranspile();

	public:
		static bool DEBUG;

		static Interpreter interpreter;
		static Transpiler transpiler;

		// Point to current exectuing code position
		static Position *pos_start;
		static Position *pos_end;

	private:
		static int runCode(const std::string &filename, const std::string &text, bool repl = false);
	};

}