$(shell mkdir -p bin)

all: bin/showpsd bin/showdif bin/benchmark

bin/showpsd: src/showpsd.cpp src/slmx4_vcom.cpp src/serialib.cpp inc/slmx4_vcom.h inc/serialib.h
	g++ -g -Wall -o bin/showpsd -I inc src/showpsd.cpp src/slmx4_vcom.cpp src/serialib.cpp

bin/showdif: src/showdif.cpp src/peak.c src/slmx4_vcom.cpp src/serialib.cpp \
			 inc/peak.h inc/slmx4_vcom.h inc/serialib.h
	g++ -g -Wall -o bin/showdif -I inc src/showdif.cpp src/peak.c src/slmx4_vcom.cpp src/serialib.cpp

bin/benchmark: src/benchmark.cpp src/slmx4_vcom.cpp src/serialib.cpp inc/slmx4_vcom.h inc/serialib.h
	g++ -g -Wall -o bin/benchmark -I inc src/benchmark.cpp src/slmx4_vcom.cpp src/serialib.cpp

clean:
	rm -rf bin
