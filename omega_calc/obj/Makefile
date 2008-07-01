# Makefile for release 1.0 of the omega system
# DON'T EDIT -- put any locally required changes in Makefile.config.
# See INSTALL for more details.


TARGET=oc

all: $(TARGET)
$(TARGET): .executable
install: install_executable
depend: depend_executable
clean: clean_self
veryclean: veryclean_self

# Required libraries
REQUIRED=code_gen omega

BASEDIR=../..

OBJS = lex.yy.o y.tab.o ${PT_OBJ} Exit.o
SRCS = ../src/parser.l ../src/parser.y $(BASEDIR)/basic/src/Exit.c
TARGET_INCL_PATH=-I.


include $(BASEDIR)/Makefile.rules
