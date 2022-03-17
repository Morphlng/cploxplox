#pragma once
#include <string>

namespace CXX {

	class Interpreter;
	class Position;

	class Runner
	{
	public:
		static int runScript(const std::string& filename);

		static int runRepl();

	public:
		static bool DEBUG;

		static Interpreter interpreter;

		// Point to current exectuing code position
		static Position* pos_start;
		static Position* pos_end;

	private:
		static int runCode(const std::string& filename, const std::string& text, bool repl = false);
	};

}