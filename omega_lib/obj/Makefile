
# Makefile for release 1.0 of the omega system
# DON'T EDIT -- put any locally required changes in Makefile.config.
# See INSTALL for more details.

LIB=omega
LIB_NAME=lib$(LIB).a
TARGET=$(LIB_NAME)

# Required libraries
REQUIRED=

all: $(TARGET)
$(TARGET): .library
install: install_library install_includes install_omega_h
depend: depend_library
clean: clean_self
veryclean: veryclean_self

# Special rule; no other library has top-level includes.
install_omega_h: always
	cd $(BASEDIR)/omega_lib/include; \
	$(CP) -p omega.h $(INCL_DESTDIR)

BASIC_SRC = $(BASEDIR)/basic/src/ConstString.c $(BASEDIR)/basic/src/Link.c
BASIC_OBJ = ConstString.o Link.o

OC_SRC = ../src/omega_core/oc.c ../src/omega_core/oc_solve.c \
         ../src/omega_core/oc_simple.c ../src/omega_core/oc_eq.c \
         ../src/omega_core/oc_problems.c ../src/omega_core/oc_print.c \
         ../src/omega_core/oc_quick_kill.c ../src/omega_core/oc_exp_kill.c \
         ../src/omega_core/oc_query.c ../src/omega_core/oc_util.c \
         ../src/omega_core/oc_global.c
OC_OBJ = oc.o oc_solve.o oc_simple.o oc_eq.o oc_problems.o oc_print.o \
         oc_query.o oc_quick_kill.o oc_exp_kill.o oc_util.o oc_global.o Exit.o

PRES_SRC = ../src/pres_print.c ../src/pres_rear.c ../src/pres_beaut.c \
           ../src/pres_dnf.c ../src/pres_conj.c ../src/pres_quant.c \
           ../src/pres_decl.c ../src/pres_logic.c ../src/pres_form.c \
           ../src/pres_cnstr.c ../src/pres_var.c ../src/pres_gen.c \
           ../src/pres_col.c ../src/pres_subs.c
PRES_OBJ = pres_print.o pres_rear.o pres_beaut.o pres_dnf.o pres_conj.o \
           pres_quant.o pres_decl.o pres_logic.o pres_form.o pres_cnstr.o \
           pres_var.o pres_gen.o pres_col.o pres_subs.o

REL_SRC = ../src/Relations.c ../src/Relation.c ../src/RelBody.c \
          ../src/RelVar.c ../src/evac.c ../src/farkas.c ../src/hull.c

REL_OBJ = Relations.o Relation.o RelBody.o RelVar.o evac.o farkas.o hull.o


FANCY_SRC = ../src/reach.c ../src/closure.c ../src/lib_hack.c \
            ../src/AST.c
FANCY_OBJ = reach.o closure.o lib_hack.o AST.o

LIB_OBJ = ${OC_OBJ} ${BASIC_OBJ} ${FANCY_OBJ} ${REL_OBJ} ${PRES_OBJ}
LIB_SRC = ${FANCY_SRC} ${REL_SRC} ${PRES_SRC} ${OC_SRC} ${BASIC_SRC}

OBJS=$(LIB_OBJ)
SRCS=$(LIB_SRC)

#Extra rule for nonstandard source layout
%.o: ../src/omega_core/%.c
	$(CC) $(CFLAGS) -c $<


BASEDIR=../..
include $(BASEDIR)/Makefile.rules
