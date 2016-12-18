default: thesis

thesis: latex/thesis.tex
	cd latex && xelatex thesis.tex && mv thesis.pdf ..

clean:
	rm -f *.fdb_latexmk
	rm -f *.log
	rm -f *.pdf
	rm -f *.aux
	rm -f *.fls
	rm -f *.out
	rm -f *.toc
