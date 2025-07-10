CC=gcc
CFLAGS=-std=gnu11 -Wall -g -O3 -pthread
LDLIBS=-lm -pthread 
JAVA=javac

EXECS=cammini.out CreaGrafo.class

all: $(EXECS) 

%.out: %.o attori.o ricerca.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c ricerca.h attori.h

	$(CC) $(CFLAGS) -c $<

%.class: %.java
	$(JAVA) $< 

clean: 
	rm *.o $(EXECS)
	
	
	
	
	
