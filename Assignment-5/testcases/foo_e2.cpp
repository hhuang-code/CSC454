/////////////////////////////////////////////////////////////////////////////
// foo2.cpp, test file for CS254, assignment 5
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdlib.h>
#include "tombstones_extension.h"

using namespace std;

void Error(const char *text)
{
    cout << "ERROR: " << text << endl;
    exit(-1);
}

int main(int argc, char **argv)
{
	Pointer<int> foo(new int[12]);  // a Pointer pointing to an array

    // subscripting
	for(int i = 0; i < 12; i++){
		foo[i] = i + 100;
	}

    // subscripting
    cout << "Print elements:" << endl;
	for(int i = 0; i < 12; i++){
		cout << foo[i] << endl;
	}

	cout << endl;

    cout << "Test add or subtract an integer:" << endl;
    // Print the first element: 100
	cout << *foo << endl;

    // Modify the second element to 91
	*(foo + 1) -= 10;

    // Print the second element: 91
	cout << *(foo + 1) << endl;

    // Print the first element: still 100
	cout << *foo << endl;

    Pointer<int> bar;
    bar = foo + 3;
    // Print the fourth element: 103
    cout << *bar << endl;

    cout << endl;

    cout << "Test comparison:" << endl;
	Pointer<int> fun;   // default constructor

	fun = foo + 4;

    // foo is smaller than fun, print: false (0)
	cout << (foo > fun) << endl; 

    Pointer<int> bat;   // default constructor

    bat = fun - 1;

    // bat is small than fun, print: true (1)
    cout << (bat < fun) << endl;

    bat.~Pointer();
    bar.~Pointer();
    fun.~Pointer();
    free(foo);

    return 0;
}
