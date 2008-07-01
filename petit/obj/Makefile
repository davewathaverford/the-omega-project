# Makefile for release 1.0 of the omega system
# DON'T EDIT -- put any locally required changes in Makefile.config.
# See INSTALL for more details.


TARGET=petit

all: $(TARGET)
$(TARGET) : .executable .tables 
	@echo Make of petit finished. # defeat the default rule
install: install_executable
# Make files from .tbl files before doing makedepend 
depend:  .tables depend_executable
depend_self:  .tables
clean: clean_self
veryclean: veryclean_self

# Required libraries, listed in order they depend on
REQUIRED=code_gen omega uniform

BASEDIR=../..

OBJt=	uniform-interf.o\
	hpp.o\
	hppy.o\
	${PT_OBJ} \
	Zima.o \
	ddomega.o\
	ddcheck.o\
	add-assert.o\
	affine.o\
	arrayExpand.o\
	ddodriver.o\
	kill.o\
	coverlist.o\
	lang-interf.o\
	ivr-lang-interf.o\
	pres-interf.o\
	ssa.o\
	ivr.o\
	vutil.o\
	depcycles.o\
	timeTrials.o\
	Exit.o\
	browse.o\
	browsedd.o\
	parse_and_analyse.o\
	copy.o\
	dddriver.o\
	ddnest.o\
        ddepsdriver.o\
	ddepsilon.o\
        ddInf.o\
        depend_filter.o\
	eps_print.o\
	ddutil.o\
	debug.o\
	compare.o\
	evaluate.o\
	f2p.o\
	language.o\
	langmenu.o\
	link.o\
	linkio.o\
	make.o\
	message.o\
	print.o\
	petit.o\
	petity.o\
	tree.o\
	notedd.o\
	build_stmt_rel.o\
	mouse.o\
	motif.o\
	system.o\
	petit_args.o\
	timer.o\
	q_stubs.o

OBJS=$(OBJt)

# SRCS grabs the includes out of the generated parsers, too
SRCS=../src/*.c *.c

# Motif, X libraries
TARGET_LIBS=$(X11_LIBS)
TARGET_INCL_PATH=-I. 
TARGET_CFLAGS=-DOMIT_QUANTIFY_CALLS

SEDS=../misc
.SUFFIXES: .val .typ .tbl

%.typ: ../src/%.tbl
	$(SED) -f ${SEDS}/tbltyp.sed < $< > $@

%.val: ../src/%.tbl
	$(SED) -f ${SEDS}/tblval.sed < $< > $@

test_lang_interf : Zima.o.li ddomega.o.li kill.o.li coverlist.o.li add-assert.o.li ivr.o.li

Zima.o.li : Zima.c lang-interf.h lang-interf.generic
	${CC} ${CFLAGS}  -c -DTEST_GENERIC_LANG_INTERF=1 ../src/Zima.c -o Zima.o.li

ddomega.o.li : ddomega.c lang-interf.h lang-interf.generic
	${CC} ${CFLAGS}  -c -DTEST_GENERIC_LANG_INTERF=1 ../src/ddomega.c -o ddomega.o.li

kill.o.li : kill.c lang-interf.h lang-interf.generic
	${CC} ${CFLAGS}  -c -DTEST_GENERIC_LANG_INTERF=1 ../src/kill.c -o kill.o.li

coverlist.o.li : coverlist.c lang-interf.h lang-interf.generic
	${CC} ${CFLAGS}  -c -DTEST_GENERIC_LANG_INTERF=1 ../src/coverlist.c -o coverlist.o.li

add-assert.o.li : add-assert.c lang-interf.h lang-interf.generic
	${CC} ${CFLAGS}  -c -DTEST_GENERIC_LANG_INTERF=1 ../src/add-assert.c -o add-assert.o.li

ivr.o.li : ivr.c ivr-lang-interf.h ivr-lang-interf.generic
	${CC} ${CFLAGS} -c -DTEST_GENERIC_LANG_INTERF=1 ../src/ivr.c -o ivr.o.li
hppy.o: hppy.c hppl.c
	${CC} $(CFLAGS) -c hppy.c

hppy.c: ../src/hppy.y
	${RUMINANT} -o hppy.c -p hpp_yy -d ../src/hppy.y

hppl.c: ../src/hppl.l
	${FLEX} -Phpp_yy -ohppl.c ../src/hppl.l

petity.c: ../src/petity.y
	${RUMINANT} -o petity.c -d ../src/petity.y

petitl.c: ../src/petitl.l
	${FLEX} -opetitl.c ../src/petitl.l

petity.o: petity.c  petitl.c
	${CC} ${CFLAGS} -c petity.c

.tables:	ddtype.typ ddtype.val exprtype.typ exprtype.val op.val op.typ symtype.typ symtype.val
	touch .tables

include $(BASEDIR)/Makefile.rules

