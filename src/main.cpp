#include "ThirdParty/argparse.h"
#include "Runner.h"
#include <string>

using namespace std;

struct MyArgs : public argparse::Args
{
	optional<string> &src_path = kwarg("f,file", "Execute Lox script from given file_path");
	bool &interactive = flag("i", "A flag to toggle interactive mode");
	bool &verbose = flag("v,verbose", "A flag to toggle verbose");
	bool &debug = flag("D,Debug", "A flag to toggle debug mode");

	void welcome() override
	{
		cout << "Welcome to Cploxplox!\n";
	}
};

void ParseArgs(int argc, char *argv[])
{
	MyArgs args = argparse::parse<MyArgs>(argc, argv);

	if (args.verbose)
		args.print();

	if (args.debug)
		CXX::Runner::DEBUG = true;

	if (args.src_path)
	{
		CXX::Runner::runScript(args.src_path.value());

		cout << "\n\nPress <Enter> to exit\n";
		cin.get();
	}
	else
	{
		args.interactive = true;
	}

	if (args.interactive)
		CXX::Runner::runRepl();
}

int main(int argc, char *argv[])
{
	if (argc == 1)
		CXX::Runner::runRepl();
	else
		ParseArgs(argc, argv);
	return 0;
}