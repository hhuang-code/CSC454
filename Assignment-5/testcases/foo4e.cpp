/////////////////////////////////////////////////////////////////////////////
// foo4.cpp, test file for CS254, assignment 5
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdlib.h>
#include "tombstones_with_switch.h"

using namespace std;

void error(const char *text)
{
    cout << "ERROR: " << text << endl;
    exit(-1);
}

int main(int argc, char **argv)
{
    Pointer<int, true> foo;
	Pointer<int, true> fun(foo);

	Pointer<int, true> bar(new int[10]);
	for (int i = 0; i < 10; i++) {
		bar[i] = 100 + i;
	}

	cout << *bar << endl;
	cout << *(bar + 1) << endl;
	cout << *bar << endl;

	foo + 1;
	fun = foo + 1;
	cout << (fun > foo) << endl;

	cout << foo - fun << endl;

    free<int, true>(foo);
    error("Didn't blow up when releasing uninitialized pointer!");

    return 0;
}
