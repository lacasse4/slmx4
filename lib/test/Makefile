$(shell mkdir -p bin)

LINC = ../inc
LSRC = ../src

all: 	bin/testzeroxing 

bin/testzeroxing: testzeroxing.cpp $(LSRC)/zeroxing.c $(LINC)/zeroxing.h
	g++ -g -Wall -o bin/testzeroxing -I $(LINC) testzeroxing.cpp $(LSRC)/zeroxing.c

clean:
	rm -rf bin
