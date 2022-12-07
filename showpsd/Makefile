$(shell mkdir -p bin)

all: bin/showpsd bin/showdif bin/benchmark bin/showdif2 bin/showrms bin/showfreq bin/testzeroxing

bin/showpsd: src/showpsd.cpp src/slmx4_vcom.cpp src/serialib.cpp inc/slmx4_vcom.h inc/serialib.h
	g++ -g -Wall -o bin/showpsd -I inc src/showpsd.cpp src/slmx4_vcom.cpp src/serialib.cpp

bin/showdif: src/showdif.cpp src/peak.c src/slmx4_vcom.cpp src/serialib.cpp \
			 inc/peak.h inc/slmx4_vcom.h inc/serialib.h
	g++ -g -Wall -o bin/showdif -I inc src/showdif.cpp src/peak.c src/slmx4_vcom.cpp src/serialib.cpp

bin/benchmark: src/benchmark.cpp src/slmx4_vcom.cpp src/serialib.cpp inc/slmx4_vcom.h inc/serialib.h
	g++ -g -Wall -o bin/benchmark -I inc src/benchmark.cpp src/slmx4_vcom.cpp src/serialib.cpp

bin/showdif2: src/showdif2.cpp src/peak.c src/slmx4_vcom.cpp src/serialib.cpp src/frameFilter.c\
			  inc/peak.h inc/slmx4_vcom.h inc/serialib.h inc/frameFilter.h
	g++ -g -Wall -o bin/showdif2 -I inc src/showdif2.cpp src/peak.c src/slmx4_vcom.cpp src/serialib.cpp src/frameFilter.c

bin/showrms: src/showrms.cpp src/slmx4_vcom.cpp src/serialib.cpp src/rms.c src/frameFilter.c \
			 inc/slmx4_vcom.h inc/serialib.h inc/rms.h inc/frameFilter.h
	g++ -g -Wall -o bin/showrms -I inc src/showrms.cpp src/slmx4_vcom.cpp src/serialib.cpp src/rms.c src/frameFilter.c

bin/showfreq: src/showfreq.cpp src/slmx4_vcom.cpp src/serialib.cpp src/rms.c src/frameFilter.c src/zeroxing.c \
			 inc/slmx4_vcom.h inc/serialib.h inc/rms.h inc/frameFilter.h inc/zeroxing.h
	g++ -g -Wall -o bin/showfreq -I inc src/showfreq.cpp src/slmx4_vcom.cpp src/serialib.cpp src/rms.c src/frameFilter.c src/zeroxing.c

bin/testzeroxing: src/testzeroxing.cpp src/zeroxing.c inc/zeroxing.h
	g++ -g -Wall -o bin/testzeroxing -I inc src/testzeroxing.cpp src/zeroxing.c

clean:
	rm -rf bin
