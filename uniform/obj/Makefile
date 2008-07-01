# Makefile for release 1.0 of the omega system
# DON'T EDIT -- put any locally required changes in Makefile.config.
# See INSTALL for more details.


LIB=uniform
LIB_NAME=lib$(LIB).a
TARGET=$(LIB_NAME)

all: $(TARGET)
$(TARGET): .library
install: install_library install_includes
depend: depend_library
clean: clean_self
veryclean: veryclean_self

# Required libraries
REQUIRED=omega code_gen

OBJS=	uniform_misc.o		communication.o		space_mappings.o \
	space_constants.o	uniform_args.o		depend_rel2.o \
	depend_dir.o		parallelism.o		time_mappings.o \
	simple_codegen.o	wak_codegen.o		search.o \
	select.o		uniform.o


SRCS=	../src/uniform_misc.c	../src/communication.c \
	../src/space_mappings.c \
	../src/space_constants.c	../src/uniform_args.c \
	../src/depend_rel2.c \
	../src/depend_dir.c	../src/parallelism.c \
	../src/time_mappings.c \
	../src/simple_codegen.c	../src/wak_codegen.c \
	../src/search.c \
	../src/select.c		../src/uniform.c

BASEDIR=../..

# Sigh.  This shouldn't really use petit includes.
PETIT=$(BASEDIR)/petit

TARGET_INCL_PATH=-I$(PETIT)/include -I$(PETIT)/obj

include $(BASEDIR)/Makefile.rules
