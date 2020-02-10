CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -g3

Unit: kdtree.o location.o kdtree_unit.o
	${CC} -o $@ ${CFLAGS} $^ -lm

kdtree_unit.o: kdtree.h location.h
location.o: location.h

clean:
	rm -r *.o Unit
