# Makefile for release 1.0 of the omega system
# DON'T EDIT -- put any locally required changes in Makefile.config.
# See INSTALL for more details.


LIB=code_gen
LIB_NAME=lib$(LIB).a
TARGET=$(LIB_NAME)

all: $(TARGET)
$(TARGET): .library
install: install_library install_includes
depend: depend_library
clean: clean_self
veryclean: veryclean_self

# Required libraries
REQUIRED=omega

BASIC_OBJ = CG_stringBuilder.o spmd.o CG_stringRepr.o code_gen.o stmt_builder.o elim.o
BASIC_SRC = ../src/CG_stringBuilder.c ../src/spmd.c ../src/CG_stringRepr.c ../src/code_gen.c ../src/stmt_builder.c ../src/elim.c

MMAP_OBJ = mmap-codegen.o mmap-util.o mmap-sub.o mmap-checks.o
MMAP_SRC = ../src/mmap-codegen.c ../src/mmap-util.c ../src/mmap-checks.c ../src/mmap-sub.c
MMAP_HEADERS = ../include/code_gen/mmap-codegen.h ../include/code_gen/mmap-util.h ../include/code_gen/mmap-checks.h ../include/code_gen/mmap-sub.h ../include/code_gen/MMap.h

OBJS=${BASIC_OBJ} ${MMAP_OBJ}
SRCS=${BASIC_SRC} ${MMAP_SRC}

BASEDIR=../..
include $(BASEDIR)/Makefile.rules


