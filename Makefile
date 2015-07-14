CFLAGS = -Wall -openmp -O3 -g

myar: myar.c
	gcc ${CFLAGS} myar.c -o myar