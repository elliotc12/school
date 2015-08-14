CC = gcc
CFLAGS = -Wall -Werror -O2 -g 
LDFLAGS = -lpthread

default: writeup.pdf

writeup.pdf: writeup.tex
	pdflatex writeup.tex

clean:
	rm -f *.aux
	rm -f *.log
	rm -f *.pdf

