# Generated automatically from Makefile.in by configure.
#### Start of system configuration section. ####

srcdir =	/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/sources/stubs
VPATH =		/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/sources/stubs

CC =		m68k-amigaos-gcc
CPP =		m68k-amigaos-gcc -E
AS =		m68k-amigaos-as

AR =		m68k-amigaos-ar
RANLIB =	m68k-amigaos-ranlib
AWK =		gawk

#### End system configuration section ####

ifneq ($(TARGET),clean)
include ../../sources/stubs/filelist
endif

CURDIR = $(shell pwd)

OPTIONS=-I$(srcdir)/../headers $(CFLAGS)

OBJECTS2=$(OBJECTS) ../nix/misc/__request.o

SUBDIRS2=$(SUBDIRS) ../nix/misc

%.o: %.c
	$(CC) $(OPTIONS) -c $^ -o $@ 2>&1|tee $*.err
	-if test ! -s $*.err; then rm $*.err; fi

.PHONY: all clean veryclean

all: libstubs.a

clean:
	rm -rf $(SUBDIRS)

veryclean:
	rm -rf *

$(foreach f,$(SUBDIRS2),$(CURDIR)/$(f)):
	mkdir $@

libstubs.a: $(foreach f,$(SUBDIRS2),$(CURDIR)/$(f)) $(OBJECTS2) ../../sources/stubs/Makefile ../../sources/stubs/filelist
	rm -f $@
	$(AR) -q $@ $(OBJECTS2)
	$(RANLIB) $@
	echo "\$$$(V)" >>$@
