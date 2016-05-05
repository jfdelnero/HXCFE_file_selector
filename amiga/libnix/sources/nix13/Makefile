# Generated automatically from Makefile.in by configure.
#### Start of system configuration section. ####

srcdir =	/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/sources/nix13
VPATH =		/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/sources/nix13

CC =		m68k-amigaos-gcc
CPP =		m68k-amigaos-gcc -E
AS =		m68k-amigaos-as

AR =		m68k-amigaos-ar
RANLIB =	m68k-amigaos-ranlib
AWK =		gawk

#### End system configuration section ####

ifneq ($(TARGET),clean)
include ../../sources/nix13/filelist
endif

CURDIR = $(shell pwd)

OPTIONS=-I$(srcdir)/../headers $(CFLAGS) -msmall-code

REDEF=-D_DOSBase=___DOSBase \
      -D_UtilityBase=___UtilityBase \
      -D_MathIeeeSingBasBase=___MathIeeeSingBasBase \
      -D_MathIeeeSingTransBase=___MathIeeeSingTransBase \
      -D_MathIeeeDoubBasBase=___MathIeeeDoubBasBase \
      -D_MathIeeeDoubTransBase=___MathIeeeDoubTransBase \
      -D_LocaleBase=___LocaleBase

%.o: %.c
	$(CC) $(OPTIONS) -S $^ -o $*.S 2>&1|tee $*.err
	$(CPP) -traditional $(REDEF) $*.S -o $*2.S
	$(CC) $(OPTIONS) $*2.S -c -o $*.o
	-rm $*.S $*2.S
	-if test ! -s $*.err; then rm $*.err; fi

.PHONY: all clean veryclean

all: libnix13.a

clean:
	-rm -rf $(SUBDIRS)

veryclean:
	-rm -rf *

$(foreach f,$(SUBDIRS),$(CURDIR)/$(f)):
	mkdir $@

libnix13.a: $(foreach f,$(SUBDIRS),$(CURDIR)/$(f)) $(OBJECTS) ../../sources/nix13/Makefile ../../sources/nix13/filelist
	-rm -f $@
	$(AR) -q $@ $(OBJECTS)
	$(RANLIB) $@
	echo "\$$$(V)" >>$@
