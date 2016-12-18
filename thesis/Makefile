default: thesis

thesis: latex/thesis.tex
	cd latex && xelatex thesis.tex && mv thesis.pdf ..

clean:
	rm -f latex/*.fdb_latexmk
	rm -f latex/*.log
	rm -f latex/*.pdf
	rm -f latex/*.aux
	rm -f latex/*.fls
	rm -f latex/*.out
	rm -f latex/*.toc
	rm -f latex/*.blg
	rm -f latex/*.bbl
