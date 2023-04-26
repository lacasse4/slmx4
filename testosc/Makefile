$(shell mkdir -p bin)

all: bin/breath1 bin/breath2

bin/breath1: src/breath1.cpp src/slmx4_vcom.cpp src/serialib.cpp src/frameFilter.c src/spatialFilter.c src/breathFilter.c src/zeroxing.c \
			 inc/slmx4_vcom.h inc/serialib.h inc/frameFilter.h inc/spatialFilter.h inc/breathFilter.h inc/zeroxing.h
	g++ -g -Wall -o bin/breath1 -I inc src/breath1.cpp src/slmx4_vcom.cpp src/serialib.cpp src/frameFilter.c src/spatialFilter.c src/breathFilter.c src/zeroxing.c

bin/breath2: src/breath2.cpp src/slmx4_vcom.cpp src/serialib.cpp src/frameFilter.c src/spatialFilter.c src/breathFilter.c src/zeroxing.c \
			 inc/slmx4_vcom.h inc/serialib.h inc/frameFilter.h inc/spatialFilter.h inc/breathFilter.h inc/zeroxing.h
	g++ -g -Wall -o bin/breath2 -I inc src/breath2.cpp src/slmx4_vcom.cpp src/serialib.cpp src/frameFilter.c src/spatialFilter.c src/breathFilter.c src/zeroxing.c

clean:
	rm -rf bin
