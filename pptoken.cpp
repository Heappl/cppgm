#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

#include "IPPTokenStream.h"
#include "DebugPPTokenStream.h"
#include "StandardData.hpp"
#include "PPTokenizer.hpp"

int main()
{
	try
	{
		ostringstream oss;
		oss << cin.rdbuf();

		string input = oss.str();

		std::shared_ptr<DebugPPTokenStream> output = std::make_shared<DebugPPTokenStream>();

		PPTokenizer tokenizer(output);

		for (char c : input)
		{
			unsigned char code_unit = c;
			tokenizer.process(code_unit);
		}

		tokenizer.process(EndOfFile);
	}
	catch (exception& e)
	{
		cerr << "ERROR: " << e.what() << endl;
		return EXIT_FAILURE;
	}
}

