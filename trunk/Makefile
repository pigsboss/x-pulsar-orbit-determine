CFLAGS=-L/usr/local/lib -L/usr/lib -I/usr/local/include -I/usr/include

ALL=orbit simulate

COPTIONS=-O3 -finput-charset=UTF8

all: $(ALL)

simulate: simulate.o satellite.o
	g++ $(CFLAGS) -o simulate simulate.o satellite.o -lgslcblas

simulate.o: simulate.cpp
	g++ $(CFLAGS) -c -o simulate.o simulate.cpp $(COPTIONS)

satellite.o: satellite.cpp satellite.h
	g++ $(CFLAGS) -c -o satellite.o satellite.cpp $(COPTIONS)

orbit: orbit.o
	g++ $(CFLAGS) -o orbit orbit.o

orbit.o: orbit.cpp
	g++ $(CFLAGS) -c -o orbit.o orbit.cpp $(COPTIONS)

clean:
	rm *.o
