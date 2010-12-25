CFLAGS=-L/usr/local/lib -L/usr/lib -L/home/azzuro/usr/lib -I/usr/local/include -I/usr/include -I/home/azzuro/usr/include

ALL=orbit simulate

COPTIONS=-O3 -finput-charset=UTF8

all: $(ALL)

simulate: simulate.o
	g++ -o simulate simulate.o -lblas

simulate.o: simulate.cpp
orbit: orbit.o
	g++ -o orbit orbit.o $(CFLAGS)

orbit.o: orbit.cpp
	g++ $(CFLAGS) -c -o orbit.o orbit.cpp $(COPTIONS)

clean:
	rm *.o
