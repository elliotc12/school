CFLAGS = -Wall -openmp -O3 -g

myar: myar.c
	icc ${CFLAGS} myar.c -o myar
