CC = gcc
CFLAGS = -Wall -Werror -O2 -g 
LDFLAGS = -pthread

default: all

all: compute writeup.pdf

compute: compute.c
	${CC} compute.c -o compute ${CFLAGS} ${LDFLAGS}

writeup.pdf: writeup.tex
	pdflatex writeup.tex

submission: 
	tar -cj -f cs344_proj5_capeke.tar.bz2 compute.c manage.py report.py writeup.tex

clean:
	rm -f \#*
	rm -f *~
	rm -f compute
	rm -f *.aux
	rm -f *.log
	rm -f *.pdf
	rm -f *.dSYM
	rm -f *.bz2
