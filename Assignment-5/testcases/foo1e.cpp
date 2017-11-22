/////////////////////////////////////////////////////////////////////////////
// foo1.cpp, test file for CS254, assignment 5
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

class Shape{
public:
	Shape(): width(0), height(0){}
	Shape(int w, int h): width(w), height(h){}
	void setWidth(int w){
		width = w;
	}
	void setHeight(int h){
		height = h;
	}
	int getWidth(){
		return width;
	}
	int getHeight(){
		return height;
	}
protected:
	int width;
	int height;
};

class Rect: public Shape{
public:
	Rect(int w, int h): Shape(w, h){}
	int getArea(){
		return width * height;
	}
};

int main(int argc, char **argv)
{
    Pointer<Shape> foo(new Shape(3, 3));

	//cout << foo->getWidth() << " " << foo->getHeight() << endl;
	cout << foo->getArea() << endl;

	free(foo);

    /*Pointer<int> tmp((int*)NULL);
    Pointer<int> bar = tmp;

	if (foo == 0)
	error("Foo shouldn't be null!");
    if (bar != 0)
	error("Bar should be null!");
    bar = new int(12);
    if (foo == bar)
	error("Foo and bar are distinct pointers!");
    if (*foo != *bar)
	error("Foo and bar should have the same value here!");

    free(foo);
    free(bar);
    cout << "foo1: OK" << endl;
	*/
	
    return 0;
}
