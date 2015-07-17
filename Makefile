CFLAGS = -Wall -openmp -O3 -g

all: myar writeup.pdf

myar: myar.c
	gcc ${CFLAGS} myar.c -o myar

writeup.pdf: writeup.tex
	pdflatex writeup.tex

clean:
	rm myar
	rm *.aux
	rm *.log