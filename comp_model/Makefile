default: crystal der

a1: latex/a1.tex
	cd latex && xelatex a1.tex && mv a1.pdf ..

a2: latex/a2.tex
	cd latex && xelatex a2.tex && mv a2.pdf ..

a4: latex/a4.tex
	cd latex && xelatex a4.tex && mv a4.pdf ..

crystal: crystal_phase.c plot.py
	gcc crystal_phase.c -g -std=c99 -lm -O2 -o crystal
	./crystal
	python2.7 plot.py

paper: latex/paper1.tex
	cd latex && xelatex paper1.tex && mv paper1.pdf ..

der: latex/der.tex
	cd latex && xelatex der.tex && mv der.pdf ..
