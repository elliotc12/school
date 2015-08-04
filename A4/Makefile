CC = icc
CFLAGS = -Wall -Werror -O2 -g -mcmodel=large
LDFLAGS = -lpthread

default: all
all: mthreads mprocs writeup.pdf

mthreads: prime_threads.c dynarr.c
	${CC} ${CFLAGS} prime_threads.c dynarr.c -o mthreads ${LDFLAGS}

mprocs: prime_procs.c dynarr.c
	${CC} ${CFLAGS} prime_procs.c dynarr.c -o mprocs ${LDFLAGS}

writeup.pdf: writeup.tex
	pdflatex writeup.tex

clean:
	rm -f mprimes
	rm -f mprocs
	rm -f *.aux
	rm -f *.log
	rm -f *.pdf
	rm -f *.dSYM
	rm -f prime_procs_temp_shm_happiness
	rm -f prime_procs_temp_shm
