/////////////////////////////////////////////////////////////////////////////
// foo2.cpp, test file for CS254, assignment 5
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdlib.h>
#include "tombstones_with_switch.h"

using namespace std;

void Error(const char *text)
{
    cout << "ERROR: " << text << endl;
    exit(-1);
}

int main(int argc, char **argv)
{
    cout << "Checking swich is off." << endl;

    cout << endl;

	Pointer<int, false> foo(new int[12]);  // a Pointer pointing to an array

    // subscripting
	for(int i = 0; i < 12; i++){
		foo[i] = i + 100;
	}

    // subscripting
    cout << "Print 12 elements:" << endl;
	for(int i = 0; i < 12; i++){
		cout << "("<< (i + 1) << ")-th element: " << foo[i] << endl;
	}

	cout << endl;

    cout << "Test add or subtract an integer:" << endl;
    // Print the first element: 100
	cout << "(1)-th element: " << *foo << endl;

    cout << "Minus the second element by 10." << endl;
    // Modify the second element to 91
	*(foo + 1) -= 10;

    // Print the second element: 91
	cout << "(2)-th element: " << *(foo + 1) << endl;

    // Print the first element: still 100
	cout << "(1)-th element: " << *foo << endl;

    Pointer<int, false> bar;
    bar = foo + 3;
    // Print the fourth element: 103
    cout << "(4)-th element: " << *bar << endl;

    cout << endl;

    cout << "Test comparison:" << endl;
	Pointer<int, false> fun;   // default constructor

	fun = foo + 4;

    // foo is smaller than fun, print: false (0)
	cout << "foo > foo + 4 true or false? " << (foo > fun) << endl; 

    Pointer<int, false> bat;   // default constructor

    bat = fun - 1;

    // bat is small than fun, print: true (1)
    cout << "fun - 1 < fun true or false? " << (bat < fun) << endl;

    bat.~Pointer();
    bar.~Pointer();
    fun.~Pointer();
    free<int, false>(foo);

    return 0;
}
