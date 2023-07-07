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

bin/extract_breath: src/extract_breath.c
	gcc -g -o bin/extract_breath src/extract_breath.c

bin/extract_breath2: src/extract_breath2.c
	gcc -g -o bin/extract_breath2 src/extract_breath2.c

bin/showfreq5: src/showfreq5.cpp \
		src/breathFilter.c \
		inc/breathFilter.h \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/zeroxing.c \
		$(LINC)/slmx4_vcom.h $(LINC)/serialib.h $(LINC)/zeroxing.h
	g++ -g -Wall -o bin/showfreq5 -I inc -I $(LINC) src/showfreq5.cpp \
		src/breathFilter.c \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/zeroxing.c

bin/breath3: src/breath3.cpp \
		src/breathFilter.c \
		inc/breathFilter.h \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/peak.c $(LSRC)/tinyosc.c \
		$(LINC)/slmx4_vcom.h $(LINC)/serialib.h $(LINC)/peak.h $(LINC)/tinyosc.h
	g++ -g -Werror -o bin/breath3 -I inc -I $(LINC) src/breath3.cpp \
		src/breathFilter.c \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/peak.c $(LSRC)/tinyosc.c

bin/fft1: src/fft1.c 
	gcc -g -Wall -o bin/fft1 src/fft1.c -lm -lfftw3f

bin/fftwrapper.o: src/fftwrapper.c inc/fftwrapper.h
	gcc -g -Wall -c -o bin/fftwrapper.o -I inc src/fftwrapper.c

bin/fft2: src/fft2.c src/fftwrapper.c inc/fftwrapper.h 
	gcc -g -Wall -o bin/fft2 -I inc src/fft2.c src/fftwrapper.c -lm -lfftw3f

bin/breath4: src/breath4.cpp \
		src/breathFilter.c \
		inc/breathFilter.h \
		bin/fftwrapper.o \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/peak.c $(LSRC)/tinyosc.c \
		$(LINC)/slmx4_vcom.h $(LINC)/serialib.h $(LINC)/peak.h $(LINC)/tinyosc.h
	g++ -g -Werror -o bin/breath4 -I inc -I $(LINC) src/breath4.cpp \
		src/breathFilter.c bin/fftwrapper.o \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/peak.c $(LSRC)/tinyosc.c \
		-lm -lfftw3f

clean:
	rm -rf bin
