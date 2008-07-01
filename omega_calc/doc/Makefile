CALCULATORSRC = calculator.tex\
		calculator-ex.tex calculator-ex2.tex calculator-ex3.tex


itright :
	make calculator.dvi; make calculator.dvi; make calculator.dvi; make calculator.dvi

calculator.ps.Z : calculator.ps
	/bin/rm -f calculator.ps.Z
	compress calculator.ps

calculator.ps : calculator.dvi
	dvips -o calculator.ps calculator.dvi

calculator.dvi : ${CALCULATORSRC} calculator.bbl calculator.uptodate
	latex2e calculator.tex
	@if egrep -i 'Latex Warning: Label\(s\) may have changed\. Rerun to get cross-references right' calculator.log > /dev/null ; then sleep 1 ; touch calculator.uptodate ; fi
	@if egrep -i 'Latex Warning: Citation .* on page [0-9]+ undefined' calculator.log > /dev/null ; then sleep 1 ; touch calculator.cites.uptodate ; fi

calculator.html: ${CALCULATORSRC}
	latex2html -address omega@cs.umd.edu -auto_navigation -t "The Omega Calculator" -reuse -show_section_numbers -split 2 calculator.tex


calculator.bbl : para.bib calculator.cites.uptodate
	touch calculator.aux ; bibtex calculator

calculator.uptodate :
	touch calculator.uptodate

calculator.cites.uptodate :
	touch calculator.cites.uptodate


para.bib :
	ln -s /fs/savoir2/p/bib/para.bib .

