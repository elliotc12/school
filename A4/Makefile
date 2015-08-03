CC = gcc
CFLAGS = -Wall -Werror -O2 -g
LDFLAGS = -lpthread

default: all
all: mthreads mprocs writeup.pdf

mthreads: prime_threads.c dynarr.c
	${CC} ${CFLAGS} prime_threads.c dynarr.c -o mthreads ${LDFLAGS}

mprocs: prime_procs.c dynarr.c
	${CC} ${CFLAGS} prime_procs.c dynarr.c -o mprocs ${LDFLAGS}

writeup.pdf: latex/writeup.tex
	cd latex && pdflatex writeup.tex

clean:
	rm -f mprimes
	rm -f mprocs
	rm -f latex/*.aux
	rm -f latex/*.log
	rm -f latex/*.pdf
	rm -f *.dSYM
