CC = gcc
CFLAGS = -Wall -Werror -O2 -g
LDFLAGS = -lpthread

default: all
all: mprimes writeup.pdf

mprimes: prime_threads.c dynarr.c
	${CC} ${CFLAGS} prime_threads.c dynarr.c -o mprimes ${LDFLAGS}

mprocs: prime_procs.c
	${CC} ${CFLAGS} prime_procs.c -o mprocs ${LDFLAGS}

writeup.pdf: latex/writeup.tex
	cd latex && pdflatex writeup.tex

clean:
	rm -f mprimes
	rm -f latex/*.aux
	rm -f latex/*.log
	rm -f latex/*.pdf
