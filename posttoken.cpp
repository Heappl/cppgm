#include <iostream>

#include "ExampleClass1.h"
#include "ExampleClass2.h"

int main()
{
	std::cout << "posttoken main" << std::endl;

	ExampleClass1 x;
	ExampleClass2 y;

	x.f();
	y.f();
}

