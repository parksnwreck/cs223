CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -g3

TSP: TSP.o location.o
	${CC} ${CFLAGS} -o TSP TSP.o location.o -lm

TSP.o: TSP.c location.h
	${CC} ${CFLAGS} -c TSP.c

location.o: location.c 
	${CC} ${CFLAGS} -c location.c
