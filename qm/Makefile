PDFs=$(patsubst %.tex, %.pdf, $(subst latex/, , $(wildcard latex/*.tex)))

default: $(PDFs)

HW%.pdf: latex/HW%.tex
	cd latex && xelatex HW$*.tex && xelatex HW$*.tex && mv HW$*.pdf ..

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
