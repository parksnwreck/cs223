CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -g3

all: Blotto Unit

Blotto: blotto.c smap.o
	${CC} ${CFLAGS} -o $@ $^ -lm

Unit: smap_unit.o smap.o smap_test_functions.o
	${CC} ${CFLAGS} -o $@ $^ -lm

smap_test_functions.o: /c/cs223/hw4/Required/smap_test_functions.c /c/cs223/hw4/Required/smap_test_functions.h
	${CC} ${CFLAGS} -c /c/cs223/hw4/Required/smap_test_functions.c

blotto.o: blotto.c /c/cs223/hw4/Required/smap.h
	${CC} ${CFLAGS} -c blotto.c

smap_unit.o: /c/cs223/hw4/Required/smap_unit.c /c/cs223/hw4/Required/smap.h /c/cs223/hw4/Required/smap_test_functions.h
	${CC} ${CFLAGS} -c /c/cs223/hw4/Required/smap_unit.c	

smap.o: smap.c /c/cs223/hw4/Required/smap.h
	${CC} ${CFLAGS} -c smap.c

clean:
	rm -r *.o Blotto Unit