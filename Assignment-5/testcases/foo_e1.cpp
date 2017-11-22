/////////////////////////////////////////////////////////////////////////////
// foo1.cpp, test file for CS254, assignment 5
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdlib.h>
#include "tombstones_extension.h"

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
    Pointer<Rect> foo(new Rect(3, 3)); // Pointer to a derived class

    // call base class function
    cout << foo->getWidth() << " " << foo->getHeight() << endl;
    // call derived class function
	cout << foo->getArea() << endl;

	free(foo);
	
    return 0;
}
