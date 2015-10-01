CC = gcc
CFLAGS = -Wall -Werror -O2 -g 
LDFLAGS = -lpthread

default: writeup.pdf submission

writeup.pdf: writeup.tex
	pdflatex writeup.tex

submission: 
	tar -cj -f cs344_final_capeke.tar.bz2 writeup.tex Makefile socket-chronology.png

clean:
	rm -f *.aux
	rm -f *.log
	rm -f *.pdf

