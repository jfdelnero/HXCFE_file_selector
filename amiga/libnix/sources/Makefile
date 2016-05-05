# Generated automatically from Makefile.in by configure.
# Built one model of the libnix libraries - use
# make CFLAGS="what you like" to built them.
#
# Combine
#
# CFLAGS=-Wall -O3
#
#   -fbaserel -DSMALL_DATA	for small data model
#   -fbaserel32			for large baserel model
#   -m68020			for 68020 specific code
#   -m68060			for 68060 specific code
#   -m68881			for 68881 specific code
#   -DIXPATHS			for Un*x path option
#   -DDEBUG_LIB 		build a library for debugging (not recommended)

#### Start of system configuration section. ####

srcdir =	/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/sources
VPATH =		/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/sources

AWK =		gawk

#### End system configuration section ####

CURDIR = $(shell pwd)

MAKELIST=$(MAKE) -f $(CURDIR)/Makefile

SUBDIRS=nixmain nix_main misc nix nix13 stack stubs

all: $(foreach f,$(SUBDIRS),$(CURDIR)/$(f))
	for subdir in $(SUBDIRS); do \
	( cd $$subdir;$(MAKE) -f ../../sources/$$subdir/Makefile V="$(V)" \
	  CFLAGS="$(CFLAGS)" ); \
	done

$(foreach f,$(SUBDIRS),$(CURDIR)/$(f)):
	mkdir $@

libamiga:
	if [ ! -d amiga ] ; then mkdir amiga ; else true ; fi
	cd amiga;$(MAKE) -f $(CURDIR)/../sources/amiga/Makefile CFLAGS="$(CFLAGS)"

filelists:
	-cd $(srcdir)/nix;	$(MAKELIST) SOURCEFILES="*/*.[cSCh]" FLDIR=$(CURDIR)/nix $(CURDIR)/nix/filelist
	-cd $(srcdir)/nix13;	$(MAKELIST) SOURCEFILES="*/*.[cSCh]" FLDIR=$(CURDIR)/nix13 $(CURDIR)/nix13/filelist
	-cd $(srcdir)/stack;	$(MAKELIST) SOURCEFILES="*.c" FLDIR=$(CURDIR)/stack $(CURDIR)/stack/filelist
	-cd $(srcdir)/stubs;	$(MAKE) -f $(CURDIR)/Makefile STUBDIR=$(CURDIR)/stubs libbases/dos.c; \
		$(MAKELIST) SOURCEFILES=" \
		libbases/*.[cSCh] misc/__DOSBase.c misc/__UtilityBase.c \
		misc/__cpucheck.c misc/__flush_cache.c misc/__initlibraries.c \
		libnames/* stubs/*.[cSCh]" \
		FLDIR=$(CURDIR)/stubs $(CURDIR)/stubs/filelist
	-cd $(srcdir)/amiga;	$(MAKELIST) SOURCEFILES="*/*.[cSCh]" FLDIR=$(CURDIR)/amiga $(CURDIR)/amiga/filelist

$(FLDIR)/filelist:
	echo "#Computer generated partial Makefile-do not edit" >$(FLDIR)/filelist
	echo "OBJECTS= \\" >>$(FLDIR)/filelist
	( for file in $(SOURCEFILES); do echo $$file; done ) | \
	$(AWK) '{ if($$0 !~ /CVS/) { print substr($$0,1,length($$0)-2) ".o \\" } }' >>$(FLDIR)/filelist
	echo >>$(FLDIR)/filelist
	echo "SUBDIRS= \\" >>$(FLDIR)/filelist
	ls -d -F *|$(AWK) '/\// { if($$0 !~ /CVS/) { print substr($$0,1,length($$0)-1) " \\" } }' >>$(FLDIR)/filelist

# build stubs for library base pointers
libbases/dos.c: $(STUBDIR)/Makefile library.list
	-rm -f libbases/* libnames/*
	$(AWK) <library.list '{ sname=substr($$2,1,length($$2)-8);        \
	  fname="libbases/" sname ".c";                      \
	  print "/* Machine-generated C-file- do not edit ! */" >fname; \
	  print "#include <stabs.h>"                            >fname; \
	  print "extern char __" sname "name[];"                >fname; \
	  print "void *" $$1 "[2]={ 0l,__" sname "name };"      >fname; \
	  print "ADD2LIB(" $$1 ");"                             >fname; \
	  fname="libnames/" sname ".c";                      \
	  print "/* Machine-generated C-file- do not edit ! */" >fname; \
	  print "char __" sname "name[]=\"" $$2 "\";"           >fname;}'

clean:
	for subdir in $(SUBDIRS); do \
	( if [ -d $$subdir ] ; then (cd $$subdir; $(MAKE) -f ../../sources/$$subdir/Makefile clean) ; else true ; fi ) \
	done

veryclean:
	-rm -rf *
