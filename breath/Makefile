$(shell mkdir -p bin)

LINC = ../lib/inc
LSRC = ../lib/src

all: 	bin/breath1 \
		bin/breath2

bin/breath1: src/breath1.cpp \
		src/frameFilter.c src/spatialFilter.c src/breathFilter.c \
		inc/frameFilter.h inc/spatialFilter.h inc/breathFilter.h \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/zeroxing.c \
		$(LINC)/slmx4_vcom.h   $(LINC)/serialib.h   $(LINC)/zeroxing.h
	g++ -g -Wall -o bin/breath1 -I inc -I $(LINC) src/breath1.cpp \
		src/frameFilter.c src/spatialFilter.c src/breathFilter.c \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/zeroxing.c

bin/breath2: src/breath2.cpp \
		src/frameFilter.c src/spatialFilter.c src/breathFilter.c \
		inc/frameFilter.h inc/spatialFilter.h inc/breathFilter.h \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/zeroxing.c $(LSRC)/tinyosc.c \
		$(LINC)/slmx4_vcom.h   $(LINC)/serialib.h   $(LINC)/zeroxing.h $(LINC)/tinyosc.h
	g++ -g -Werror -o bin/breath2 -I inc -I $(LINC) src/breath2.cpp \
		src/frameFilter.c src/spatialFilter.c src/breathFilter.c \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/zeroxing.c $(LSRC)/tinyosc.c

clean:
	rm -rf bin
