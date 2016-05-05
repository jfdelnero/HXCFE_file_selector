# Generated automatically from Makefile.in by configure.
#### Start of system configuration section. ####

srcdir =	/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/sources/math
VPATH =		/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/sources/math

CC =		m68k-amigaos-gcc
CPP =		m68k-amigaos-gcc -E
AS =		m68k-amigaos-as

AR =		m68k-amigaos-ar
RANLIB =	m68k-amigaos-ranlib
AWK =		gawk

#### End system configuration section ####

ifneq ($(TARGET),clean)
include ../../sources/math/filelist
endif

CURDIR = $(shell pwd)

OPTIONS=-I$(srcdir)/../headers -DFULL_SPECIFIERS $(CFLAGS)

OBJECTS2=$(OBJECTS) \
         ../nix/assert/__eprintf.o \
         ../nix/stdio/printf.o ../nix/stdio/fprintf.o ../nix/stdio/sprintf.o \
         ../nix/stdio/vprintf.o ../nix/stdio/vsprintf.o stdio/vfprintf.o \
         ../nix/stdio/scanf.o ../nix/stdio/fscanf.o ../nix/stdio/sscanf.o \
         ../nix/stdio/vscanf.o ../nix/stdio/vsscanf.o stdio/vfscanf.o \
         ../nix/stdio/tmpfile.o ../nix/stdio/tmpnam.o

SUBDIRS2=$(SUBDIRS) ../nix/assert ../nix/stdio

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

all: libm.a

clean:
	-rm -rf $(SUBDIRS2)

veryclean:
	-rm -rf *

$(foreach f,$(SUBDIRS2),$(CURDIR)/$(f)):
	mkdir $@

libm.a: $(foreach f,$(SUBDIRS2),$(CURDIR)/$(f)) $(OBJECTS2) ../../sources/math/Makefile ../../sources/math/filelist
	-rm -f $@
	$(AR) -q $@ $(OBJECTS2)
	$(RANLIB) $@
	echo "\$$$(V)" >>$@

stdio/vfprintf.o: $(srcdir)/../nix/stdio/vfprintf.c
	$(CC) $(OPTIONS) $^ -c -o stdio/vfprintf.o

stdio/vfscanf.o: $(srcdir)/../nix/stdio/vfscanf.c
	$(CC) $(OPTIONS) $^ -c -o stdio/vfscanf.o
