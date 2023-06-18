$(shell mkdir -p bin)

LINC = ../lib/inc
LSRC = ../lib/src

all: 	bin/extract_breath \
		bin/extract_breath2

bin/extract_breath: src/extract_breath.c
	gcc -g -o bin/extract_breath src/extract_breath.c

bin/extract_breath2: src/extract_breath2.c
	gcc -g -o bin/extract_breath2 src/extract_breath2.c

clean:
	rm -rf bin
