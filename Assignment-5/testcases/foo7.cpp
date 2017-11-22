/////////////////////////////////////////////////////////////////////////////
// foo7.cpp, test file for CS254, assignment 5
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdlib.h>
#include "tombstones.h"

using namespace std;

void error(const char *text)
{
    cout << "ERROR: " << text << endl;
    exit(-1);
}

void sub(Pointer<int> &foo)
{
    Pointer<int> bar(foo);	// copy constructor
    Pointer<int> bat(bar);	// copy constructor
    Pointer<int> qix(bat);	// copy constructor
    Pointer<int> glorch(qix);	// copy constructor

    *glorch = 100;	// foo, bar, bat, qix and glorch point to the same tombstone
    if (*foo != 100)
		error("Linking of pointers not correct!");

	//bar.~Pointer();
	//bat.~Pointer();
	//qix.~Pointer();
	//glorch.~Pointer();
	//foo.~Pointer();
    free(glorch);	// dangling pointer. To avoid this, you should call arbitrary four destructors. If five are called, memory leak will happen.
}

int main(int argc, char **argv)
{
    Pointer<int> foo(new int(0));	// bootstrapping constructor
    sub(foo);
    *foo = 1000;
    error("Didn't complain about use of dangling pointer foo!");

    return 0;
}
