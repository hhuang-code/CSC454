# Type "make all" to build two targets: 'parse' for exception-based recovery and 'iedparse' for immediate error detection.
# Type "make test" to test 'parse' program, and type 'iedparse' to test 'iedtest' program
# Test case is provided in 'case.data' file, and the original test case is a correct program, you can make any change to it (e.g., delete a '*' character anywhere to make it wrong).

iedtest:
	./iedparse < case.data

test:
	./parse < case.data

all: parse iedparse

iedparse: iedparse.o scan.o
	g++ -o iedparse iedparse.o scan.o

parse: parse.o scan.o
	g++ -o parse parse.o scan.o

clean:
	rm *.o parse iedparse

iedparse.o: iedparse.cpp scan.hpp
	g++ -c iedparse.cpp

parse.o: parse.cpp scan.hpp
	g++ -c parse.cpp

scan.o: scan.cpp scan.hpp
	g++ -c scan.cpp
