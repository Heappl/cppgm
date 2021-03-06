// (C) 2013 CPPGM Foundation www.cppgm.org.  All rights reserved.

#include "DebugPostTokenOutputStream.h"
#include "DebugPPTokenStream.h"
#include "PostTokenAnalyser.hpp"
#include "PPTokenizer.hpp"
#include <iostream>
#include <sstream>
#include <fstream>

int main()
{
	// TODO:
	// 1. apply your code from PA1 to produce `preprocessing-tokens`
	// 2. "post-tokenize" the `preprocessing-tokens` as described in PA2
	// 3. write them out in the PA2 output format specifed

	// You may optionally use the above starter code.
	//
	// In particular there is the DebugPostTokenOutputStream class which helps form the
	// correct output format:

	std::shared_ptr<IPostTokenStream> output = std::make_shared<DebugPostTokenOutputStream>();
    std::shared_ptr<IPPTokenStream> postTokenAnalyser = std::make_shared<PostTokenAnalyser>(output);
    PPTokenizer tokenizer(postTokenAnalyser);

	try
	{
		ostringstream oss;
		oss << cin.rdbuf();

		string input = oss.str();

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
