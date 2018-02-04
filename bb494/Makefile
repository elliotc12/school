SVG_FIGURES = $(wildcard figures/*.svg)
FIGURES = $(patsubst %.svg,%.pdf,$(SVG_FIGURES))

manuscript.pdf: manuscript.tex references.bib $(FIGURES)
	xelatex manuscript.tex && bibtex manuscript && xelatex manuscript.tex && xelatex manuscript.tex

figures/%.pdf: figures/%.svg
	inkscape -D --export-pdf $(shell pwd)/$@ $(shell pwd)/$<
