# Makefile for release 1.0 of the omega system
# DON'T EDIT -- put any locally required changes in Makefile.config.
# See INSTALL for more details.

# Top-level makefile: lists possible targets, call those makefiles, install
#  target

default:
	@echo Valid targets are:
	@echo "  " executables: oc petit
	@echo "  " libraries: libomega.a libcode_gen.a libuniform.a
	@echo "  " all: all above targets
	@echo Or, change to the appropriate subdirectory and do \"make\".
	@echo In any directory, BE SURE to do a \"make depend\" before compiling!
	@echo Edit Makefile.config to customize\; see INSTALL for details.

all: oc petit

BASEDIR =.

oc: always
	cd omega_calc/obj; $(MAKE) oc

libparse.a: 
	cd parse_lib/obj; $(MAKE) libparse.a

tables: 
	cd petit/obj; $(MAKE) .tables

petit: tables always
	cd petit/obj; $(MAKE) petit

rtrt-petit: tables always
	cd petit/obj; $(MAKE) -f Makefile-rtrt rtrt-petit

libomega.a: always 
	cd omega_lib/obj; $(MAKE) libomega.a

libcode_gen.a: always
	cd code_gen/obj; $(MAKE) libcode_gen.a

libuniform.a: tables always 
	cd uniform/obj; $(MAKE) libuniform.a

install: install_all
depend: depend_all
clean: clean_all
veryclean: veryclean_all

SUBMAKE=$(MAKE)

include $(BASEDIR)/Makefile.rules
