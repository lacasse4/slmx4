$(shell mkdir -p bin)

LINC = ../lib/inc
LSRC = ../lib/src

all: 	bin/showpsd \
		bin/showdif \
		bin/benchmark \
		bin/showfilter \
		bin/showrms \
		bin/showfreq \
		bin/showfreq2 \
		bin/showfreq3 \
		bin/showfreq3_dbg \
		bin/showfreq4 

bin/showpsd: src/showpsd.cpp \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp \
		$(LINC)/slmx4_vcom.h   $(LINC)/serialib.h
	g++ -g -Wall -o bin/showpsd -I $(LINC) src/showpsd.cpp $(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp

bin/showdif: src/showdif.cpp \
		$(LSRC)/peak.c $(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp \
		$(LINC)/peak.h $(LINC)/slmx4_vcom.h   $(LINC)/serialib.h
	g++ -g -Wall -o bin/showdif -I $(LINC) src/showdif.cpp $(LSRC)/peak.c $(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp

bin/benchmark: src/benchmark.cpp \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp \
		$(LINC)/slmx4_vcom.h   $(LINC)/serialib.h
	g++ -g -Wall -o bin/benchmark -I $(LINC) src/benchmark.cpp $(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp

bin/showfilter: src/showfilter.cpp src/frameFilter.c inc/frameFilter.h \
		$(LSRC)/peak.c $(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp \
		$(LINC)/peak.h $(LINC)/slmx4_vcom.h   $(LINC)/serialib.h 
	g++ -g -Wall -o bin/showfilter -I inc -I $(LINC) src/showfilter.cpp src/frameFilter.c \
		$(LSRC)/peak.c $(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp 

bin/showrms: src/showrms.cpp src/frameFilter.c inc/frameFilter.h \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/rms.c \
		$(LINC)/slmx4_vcom.h   $(LINC)/serialib.h   $(LINC)/rms.h
	g++ -g -Wall -o bin/showrms -I inc -I $(LINC) src/showrms.cpp src/frameFilter.c \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/rms.c 

bin/showfreq: src/showfreq.cpp src/frameFilter.c inc/frameFilter.h \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/rms.c $(LSRC)/zeroxing.c \
		$(LINC)/slmx4_vcom.h   $(LINC)/serialib.h   $(LINC)/rms.h $(LINC)/zeroxing.h
	g++ -g -Wall -o bin/showfreq -I inc -I $(LINC) src/showfreq.cpp src/frameFilter.c \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/rms.c $(LSRC)/zeroxing.c

bin/showfreq2: src/showfreq2.cpp src/frameFilter.c inc/frameFilter.h \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/rms.c $(LSRC)/zeroxing.c \
		$(LINC)/slmx4_vcom.h   $(LINC)/serialib.h   $(LINC)/rms.h $(LINC)/zeroxing.h
	g++ -g -Wall -o bin/showfreq2 -I inc -I $(LINC) src/showfreq2.cpp src/frameFilter.c \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/rms.c $(LSRC)/zeroxing.c

bin/showfreq3: src/showfreq3.cpp src/frameFilter.c inc/frameFilter.h \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/zeroxing.c \
		$(LINC)/slmx4_vcom.h   $(LINC)/serialib.h   $(LINC)/zeroxing.h
	g++ -g -Wall -o bin/showfreq3 -I inc -I $(LINC) src/showfreq3.cpp src/frameFilter.c \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/zeroxing.c

bin/showfreq3_dbg: src/showfreq3_dbg.cpp
	g++ -g -Wall -o bin/showfreq3_dbg src/showfreq3_dbg.cpp

bin/showfreq4: src/showfreq4.cpp \
		src/frameFilter.c src/spatialFilter.c src/breathFilter.c \
		inc/frameFilter.h inc/spatialFilter.h inc/breathFilter.h \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/zeroxing.c \
		$(LINC)/slmx4_vcom.h $(LINC)/serialib.h $(LINC)/zeroxing.h
	g++ -g -Wall -o bin/showfreq4 -I inc -I $(LINC) src/showfreq4.cpp \
		src/frameFilter.c src/spatialFilter.c src/breathFilter.c \
		$(LSRC)/slmx4_vcom.cpp $(LSRC)/serialib.cpp $(LSRC)/zeroxing.c

clean:
	rm -rf bin
