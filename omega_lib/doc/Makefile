
INTERFACESRC = interface.tex tex_stuff.tex compile.tex\
		concepts.tex data-structures.tex \
		ex-1.tex ex-2.tex ex-3.tex ex-4.tex ex-5.tex \
		construction.tex querying.tex \
		working_with.tex \
		core.tex \
		simplify.tex \
		misc.tex \
		reference.tex\
		uniform.tex

itright :
	make interface.dvi; make interface.dvi; make interface.dvi; make interface.dvi

interface.html: ${INTERFACESRC}
	latex2html -address omega@cs.umd.edu -auto_navigation -t "The Omega Library Interface Guide" -reuse -show_section_numbers -split 2 interface.tex

fast:	interface.dvi

interface.ps.Z : interface.ps
	/bin/rm -f interface.ps.Z
	compress interface.ps

interface.ps : interface.dvi
	dvips -o interface.ps interface.dvi

interface.dvi : ${INTERFACESRC} interface.bbl interface.uptodate
	latex interface.tex
	@if egrep 'LaTeX Warning: Label\(s\) may have changed\. Rerun to get cross-references right' interface.log > /dev/null ; then sleep 1 ; touch interface.uptodate ; fi
	@if egrep 'LaTeX Warning: Citation .* on page [0-9]+ undefined' interface.log > /dev/null ; then sleep 1 ; touch interface.cites.uptodate ; fi

interface.bbl : para.bib interface.cites.uptodate
	touch interface.aux ; bibtex interface

tech-title: tech-title.ps

tech-title.ps: tech-title.dvi
	dvips tech-title

tech-title.dvi: tech-title.tex tech-abstract.tex
	latex tech-title



para.bib :
	ln -s /fs/savoir2/p/bib/para.bib .


# make these if they don't exist yet

interface.uptodate :
	touch interface.uptodate

interface.cites.uptodate :
	touch interface.cites.uptodate



EXAMPLE = library_example.c

examples: ex-1.tex  ex-2.tex ex-3.tex ex-4.tex ex-5.tex

ex-1.tex : ${EXAMPLE}
	sed -n -e '/BEGIN PART 1/,/END PART 1/p' ${EXAMPLE} | sed -e '1s/.*/\\begin{verbatim}/' -e '$$s/.*/\\end{verbatim}/' > ex-1.tex

ex-2.tex : ${EXAMPLE}
	sed -n -e '/BEGIN PART 2/,/END PART 2/p' ${EXAMPLE} | sed -e '1s/.*/\\begin{verbatim}/' -e '$$s/.*/\\end{verbatim}/' > ex-2.tex

ex-3.tex : ${EXAMPLE}
	sed -n -e '/BEGIN PART 3/,/END PART 3/p' ${EXAMPLE} | sed -e '1s/.*/\\begin{verbatim}/' -e '$$s/.*/\\end{verbatim}/' > ex-3.tex

ex-4.tex : ${EXAMPLE}
	sed -n -e '/BEGIN PART 4/,/END PART 4/p' ${EXAMPLE} | sed -e '1s/.*/\\begin{verbatim}/' -e '$$s/.*/\\end{verbatim}/' > ex-4.tex

ex-5.tex : ${EXAMPLE}
	sed -n -e '/BEGIN PART 5/,/END PART 5/p' ${EXAMPLE} | sed -e '1s/.*/\\begin{verbatim}/' -e '$$s/.*/\\end{verbatim}/' > ex-5.tex

# test example code

CC = g++263
PURIFY =
# PURIFY = purify -collector=/usr/imports/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.2/ld

PT-example.o : PT-example.c ../include/omega/PT-omega.c
	${CC} -g -w -I../include -I../../basic/include -fno-implicit-templates -DDONT_INCLUDE_TEMPLATE_CODE -DDONT_DEFINE_BOOL -c PT-example.c

example.o : example.c
	${CC} -g -Wall -I../include -I../../basic/include -fno-implicit-templates -DDONT_INCLUDE_TEMPLATE_CODE -DDONT_DEFINE_BOOL -c example.c

example : example.o PT-example.o ../obj/libomega.a ConstString.o Exit.o Link.o
	${PURIFY} ${CC} -g -I../include -I../../basic/include -fno-implicit-templates -DDONT_INCLUDE_TEMPLATE_CODE -DDONT_DEFINE_BOOL example.o PT-example.o -L../obj ConstString.o Exit.o Link.o -lomega -o example

library_example.o : library_example.c
	${CC} -Wall -g -I../include -I../../basic/include -fno-implicit-templates -DDONT_INCLUDE_TEMPLATE_CODE -DDONT_DEFINE_BOOL -c library_example.c

library_example : library_example.o PT-example.o ../obj/libomega.a ConstString.o Exit.o Link.o
	${PURIFY} ${CC} -g -I../include -I../../basic/include -fno-implicit-templates -DDONT_INCLUDE_TEMPLATE_CODE -DDONT_DEFINE_BOOL library_example.o PT-example.o -L../obj ConstString.o Exit.o Link.o -lomega -o library_example

bad.o : bad.c
	${CC} -Wall -g -I../include -I../../basic/include -fno-implicit-templates -DDONT_INCLUDE_TEMPLATE_CODE -DDONT_DEFINE_BOOL -c bad.c

bad : bad.o PT-example.o ../obj/libomega.a ConstString.o Exit.o Link.o
	${PURIFY} ${CC} -g -I../include -I../../basic/include -fno-implicit-templates -DDONT_INCLUDE_TEMPLATE_CODE -DDONT_DEFINE_BOOL bad.o PT-example.o -L../obj ConstString.o Exit.o Link.o -lomega -o bad

Link.o: ../../basic/src/Link.c
	${CC} -Wall -g -I../include -I../../basic/include -fno-implicit-templates -DDONT_INCLUDE_TEMPLATE_CODE -DDONT_DEFINE_BOOL -c ../../basic/src/Link.c

ConstString.o: ../../basic/src/ConstString.c
	${CC} -Wall -g -I../include -I../../basic/include -fno-implicit-templates -DDONT_INCLUDE_TEMPLATE_CODE -DDONT_DEFINE_BOOL -c ../../basic/src/ConstString.c

Exit.o: ../../basic/src/Exit.c
	${CC} -Wall -g -I../include -I../../basic/include -fno-implicit-templates -DDONT_INCLUDE_TEMPLATE_CODE -DDONT_DEFINE_BOOL -c ../../basic/src/Exit.c

