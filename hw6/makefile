CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -g3

all: TSP Unit

Unit: lugraph.o location.o lugraph_unit.o
	${CC} -o $@ ${CFLAGS} $^ -lm

TSP: TSP.o lugraph.o location.o
	${CC} -o $@ ${CFLAGS} $^ -lm

TSP.o: TSP.c

lugraph_unit.o: lugraph_unit.c lugraph.h location.h

lugraph.o: lugraph.c

location.o: location.h

clean:
	rm -r *.o Unit
