CFLAGS = -Wall -openmp -O3 -g

all: myar writeup.pdf

myar: myar.c
	icc ${CFLAGS} myar.c -o myar

writeup.pdf: writeup.tex
	pdflatex writeup.tex

clean:
	rm -f myar
	rm -f *.aux
	rm -f *.log
