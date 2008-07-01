#
#	Makefile for f2p, a Fortran 77 to petit converter
#

CC = gcc
CFLAGS = -g
LDFLAGS = 
# CFLAGS = -O2

SHELL = /bin/sh

OBJECTSd = main.o init.o gram.o lex.o proc.o equiv.o data.o format.o \
	  expr.o exec.o intr.o io.o misc.o error.o mem.o names.o \
	  output.o p1output.o put.o putpcc.o vax.o formatdata.o \
	  parse_args.o niceprintf.o cds.o sysdep.o 
OBJECTS = $(OBJECTSd)

f2p: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o f2p

gram.c:	gram.head gram.dcl gram.expr gram.exec gram.io defs.h tokdefs.h
	( sed <tokdefs.h "s/#define/%token/" ;\
		cat gram.head gram.dcl gram.expr gram.exec gram.io ) >gram.in
	$(YACC) $(YFLAGS) gram.in
	echo "(expect 4 shift/reduce)"
	sed 's/^# line.*/\/* & *\//' y.tab.c >gram.c
	rm -f gram.in y.tab.c

$(OBJECTSd): defs.h ftypes.h defines.h machdefs.h sysdep.h

tokdefs.h: tokens
	grep -n . <tokens | sed "s/\([^:]*\):\(.*\)/#define \2 \1/" >tokdefs.h

clean:
	rm -f gram.c *.o f2p tokdefs.h y.tab.h

depend: gram.c
	makedepend *.c


# DO NOT DELETE THIS LINE -- make depend depends on it.

cds.o: sysdep.h /usr/include/stdio.h /usr/include/string.h
cds.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h
data.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
data.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
data.o: machdefs.h
equiv.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
equiv.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
equiv.o: machdefs.h
error.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
error.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
error.o: machdefs.h
exec.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
exec.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
exec.o: machdefs.h p1defs.h names.h
expr.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
expr.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
expr.o: machdefs.h output.h niceprintf.h names.h
format.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
format.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
format.o: machdefs.h p1defs.h format.h output.h niceprintf.h names.h iob.h
formatdata.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
formatdata.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h
formatdata.o: defines.h machdefs.h output.h niceprintf.h names.h format.h
gram.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
gram.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
gram.o: machdefs.h p1defs.h /usr/include/malloc.h /usr/include/memory.h
gram.o: /usr/include/values.h
init.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
init.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
init.o: machdefs.h output.h niceprintf.h iob.h
intr.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
intr.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
intr.o: machdefs.h names.h
io.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
io.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
io.o: machdefs.h names.h iob.h
lex.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
lex.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
lex.o: machdefs.h tokdefs.h p1defs.h
main.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
main.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
main.o: machdefs.h parse.h
mem.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
mem.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
mem.o: machdefs.h iob.h
misc.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
misc.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
misc.o: machdefs.h
names.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
names.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
names.o: machdefs.h output.h niceprintf.h names.h iob.h
niceprintf.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
niceprintf.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h
niceprintf.o: defines.h machdefs.h names.h output.h niceprintf.h
output.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
output.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
output.o: machdefs.h names.h output.h niceprintf.h
p1output.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
p1output.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h
p1output.o: defines.h machdefs.h p1defs.h output.h niceprintf.h names.h
parse_args.o: /usr/include/stdio.h parse.h /usr/include/math.h
parse_args.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
parse_args.o: /usr/include/ctype.h
proc.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
proc.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
proc.o: machdefs.h names.h output.h niceprintf.h p1defs.h
put.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
put.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
put.o: machdefs.h names.h pccdefs.h p1defs.h
putpcc.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
putpcc.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
putpcc.o: machdefs.h pccdefs.h output.h niceprintf.h names.h p1defs.h
sysdep.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
sysdep.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
sysdep.o: machdefs.h usignal.h /usr/include/signal.h
sysdep.o: /usr/include/sys/signal.h /usr/include/vm/faultcode.h
vax.o: defs.h sysdep.h /usr/include/stdio.h /usr/include/string.h
vax.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h ftypes.h defines.h
vax.o: machdefs.h pccdefs.h output.h niceprintf.h
