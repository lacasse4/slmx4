$(shell mkdir -p bin)

LINC = ../lib/inc
LSRC = ../lib/src

all: 	bin/extract_breath \
		bin/extract_breath2 \
		bin/showfreq5 \
		bin/breath3 \
		bin/fft1 \
		bin/fft2 \
		bin/breath4 

bin/tinyosc.o: $(LSRC)/tinyosc.c $(LINC)/tinyosc.h
	gcc -g -Werror -c -o bin/tinyosc.o -I $(LINC) $(LSRC)/tinyosc.c

bin/fftwrapper.o: src/fftwrapper.c inc/fftwrapper.h
	gcc -g -Wall -c -o bin/fftwrapper.o -I inc src/fftwrapper.c

bin/extract_breath: src/extract_breath.c
	gcc -g -o bin/extract_breath src/extract_breath.c

bin/extract_breath2: src/extract_breath2.c
	gcc -g -o bin/extract_breath2 src/extract_breath2.c

bin/showfreq5: src/showfreq5.cpp \
		src/breathFilter.c inc/breathFilter.h \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/zeroxing.c \
		$(LINC)/slmx4_vcom.h $(LINC)/serialib.h $(LINC)/zeroxing.h
	g++ -g -Wall -o bin/showfreq5 -I inc -I $(LINC) src/showfreq5.cpp \
		src/breathFilter.c \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/zeroxing.c

bin/breath3: src/breath3.cpp src/breathFilter.c inc/breathFilter.h \
		bin/tinyosc.o \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/peak.c  \
		$(LINC)/slmx4_vcom.h $(LINC)/serialib.h $(LINC)/peak.h
	g++ -g -Werror -o bin/breath3 -I inc -I $(LINC) src/breath3.cpp \
		src/breathFilter.c bin/tinyosc.o \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/peak.c

bin/fft1: src/fft1.c 
	gcc -g -Wall -o bin/fft1 src/fft1.c -lm -lfftw3f

bin/fft2: src/fft2.c bin/fftwrapper.o 
	gcc -g -Wall -o bin/fft2 -I inc src/fft2.c bin/fftwrapper.o -lm -lfftw3f

bin/breath4: src/breath4.cpp bin/fftwrapper.o bin/tinyosc.o \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/peak.c $(LSRC)/timer_us.c $(LSRC)/buffer.c \
		$(LINC)/slmx4_vcom.h $(LINC)/serialib.h $(LINC)/peak.h $(LINC)/timer_us.h $(LINC)/buffer.h
	g++ -g -Wall -o bin/breath4 -I inc -I $(LINC) src/breath4.cpp \
		bin/fftwrapper.o bin/tinyosc.o \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/peak.c $(LSRC)/timer_us.c $(LSRC)/buffer.c \
		-lm -lfftw3f

clean:
	rm -rf bin
