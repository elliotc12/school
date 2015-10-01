CFLAGS = -Wall -openmp -O3 -g

all: mysh writeup.pdf

mysh: mysh.c
	gcc ${CFLAGS} mysh.c -o mysh

writeup.pdf: latex/writeup.tex
	pdflatex latex/writeup.tex

clean:
	rm -f mysh
	rm -f *.aux
	rm -f *.log
