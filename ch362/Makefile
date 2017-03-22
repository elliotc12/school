PDFs=$(patsubst %.tex, %.pdf, $(subst latex/, , $(wildcard latex/*.tex)))

default: $(PDFs)

%.pdf: latex/%.tex
	cd latex && xelatex $*.tex && xelatex $*.tex && mv $*.pdf ..

clean:
	rm -f *.pdf
	rm -f *~
	rm -f latex/*.log
	rm -f latex/*.aux
	rm -f latex/*.out
	rm -f *.log
	rm -f *.aux
	rm -f *.out
	rm -f *.fls
	rm -f *.fdb_latexmk
