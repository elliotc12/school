default: thesis.pdf

FIGURES=$(patsubst figures/%.svg, figures/%.pdf, $(wildcard figures/*.svg))

thesis.pdf: latex/thesis.tex $(FIGURES)
	cd latex && xelatex thesis.tex && bibtex thesis && xelatex thesis.tex && xelatex thesis.tex && mv thesis.pdf ..

continuous:
	cd latex && latexmk -pvc --xelatex thesis.tex

figures/%.pdf: figures/%.svg
	inkscape -D --export-pdf $@ $<

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

##### 403 Stuff #####

tate-figures-1-17.pdf: latex/tate-figures-1-17.tex $(FIGURES)
	cd latex && xelatex tate-figures-1-17.tex && mv tate-figures-1-17.pdf ..
