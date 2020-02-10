CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -g3

all: Heatmap Unit

Heatmap: heatmap.c track.o trackpoint.o location.o
	${CC} ${CFLAGS} -lm -o Heatmap heatmap.c track.o trackpoint.o location.o

Unit: track_unit.c track.o trackpoint.o location.o
	${CC} ${CFLAGS} -lm -o Unit track_unit.c track.o trackpoint.o location.o

track.o: track.c track.h trackpoint.h
	${CC} ${CFLAGS} -c track.c

trackpoint.o: trackpoint.c trackpoint.h
	${CC} ${CFLAGS} -c trackpoint.c

location.o: location.c location.h
	${CC} ${CFLAGS} -c location.c

heatmap.o: heatmap.c trackpoint.h
	${CC} ${CFLAGS} -c heatmap.c