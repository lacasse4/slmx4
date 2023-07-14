$(shell mkdir -p bin)

LINC = ../inc
LSRC = ../src

all: 	bin/testzeroxing bin/testpeak

bin/testzeroxing: testzeroxing.c $(LSRC)/zeroxing.c $(LINC)/zeroxing.h
	gcc -g -Wall -o bin/testzeroxing -I $(LINC) testzeroxing.c $(LSRC)/zeroxing.c

bin/testpeak: testpeak.c $(LSRC)/peak.c $(LINC)/peak.h
	gcc -g -Wall -o bin/testpeak -I $(LINC) testpeak.c $(LSRC)/peak.c -lm

clean:
	rm -rf bin
