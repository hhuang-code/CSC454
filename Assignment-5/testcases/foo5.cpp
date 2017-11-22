/////////////////////////////////////////////////////////////////////////////
// foo5.cpp, test file for CS254, assignment 5
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

void proc()
{
    Pointer<int> foo(new int(12));  // bootstrapping constructor 
}

int main(int argc, char **argv)
{
    proc();     // After finishing proc(), foo.~Pointer() will be called; but the object (new int(12)) has not been freed.
    error("Didn't blow up when leaking memory!");

    return 0;
}
