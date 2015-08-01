CC = gcc
CFLAGS = -Wall -Werror -O2 -g
LDFLAGS = -lpthread
SOURCES = primes.c dynarr.c

default: all
all: mprimes writeup.pdf

mprimes: prime_threads.c
	${CC} ${CFLAGS} prime_threads.c -o mprimes ${LDFLAGS}

writeup.pdf: latex/writeup.tex
	cd latex && pdflatex writeup.tex

clean:
	rm -f mprimes
	rm -f latex/*.aux
	rm -f latex/*.log
	rm -f latex/*.pdf
