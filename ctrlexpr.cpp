// (C) 2013 CPPGM Foundation www.cppgm.org.  All rights reserved.

#include "DebugControlExpression.h"
#include "ControlExpressionEvaluator.hpp"
#include "PostTokenAnalyser.hpp"
#include "PPTokenizer.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

using namespace std;

// mock implementation of IsDefinedIdentifier for PA3
// return true iff first code point is odd
bool PA3Mock_IsDefinedIdentifier(const string& identifier)
{
	if (identifier.empty())
		return false;
	else
		return identifier[0] % 2;
}

int main()
{
    std::shared_ptr<IControlExpressionEvaluator> ctrlExprEvaluator = std::make_shared<ControlExpressionEvaluator>();
    std::shared_ptr<IPostTokenStream> output = std::make_shared<DebugControlExpressionEvaluatorProxy>(ctrlExprEvaluator);
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

