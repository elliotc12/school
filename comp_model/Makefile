default: crystal

a1: latex/a1.tex
	cd latex && xelatex a1.tex && mv a1.pdf ..

a2: latex/a2.tex
	cd latex && xelatex a2.tex && mv a2.pdf ..

a4: latex/a4.tex
	cd latex && xelatex a4.tex && mv a4.pdf ..

crystal: crystal_phase.c
	gcc crystal_phase.c -g -o crystal

