$(shell mkdir -p bin)

bin/main: src/main.cpp src/slmx4_vcom.cpp src/serialib.cpp inc/slmx4_vcom.h inc/serialib.h
	g++ -g -Wall -o bin/main -I inc src/main.cpp src/slmx4_vcom.cpp src/serialib.cpp

clean:
	rm -rf bin
