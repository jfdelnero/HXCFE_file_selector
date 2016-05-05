# Generated automatically from Makefile.in by configure.
#### Start of system configuration section. ####

srcdir =	/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/sources/nix_main
VPATH =		/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/sources/nix_main

CC =		m68k-amigaos-gcc
CPP =		m68k-amigaos-gcc -E
AS =		m68k-amigaos-as

AR =		m68k-amigaos-ar
RANLIB =	m68k-amigaos-ranlib
AWK =		gawk

#### End system configuration section ####

OBJECTS=__nocommandline.o _main.o

OPTIONS=-I$(srcdir)/../headers $(CFLAGS)

REDEF=-D_DOSBase=___DOSBase \
      -D_UtilityBase=___UtilityBase \
      -D_MathIeeeSingBasBase=___MathIeeeSingBasBase \
      -D_MathIeeeSingTransBase=___MathIeeeSingTransBase \
      -D_MathIeeeDoubBasBase=___MathIeeeDoubBasBase \
      -D_MathIeeeDoubTransBase=___MathIeeeDoubTransBase \
      -D_LocaleBase=___LocaleBase

#Use private library bases to avoid naming collisions
%.o: %.c
	$(CC) $(OPTIONS) -S $^ -o $*.S 2>&1|tee $*.err
	$(CPP) -traditional $(REDEF) $*.S -o $*2.S
	$(CC) $(OPTIONS) $*2.S -c -o $*.o
	-rm $*.S $*2.S
	-if test ! -s $*.err; then rm $*.err; fi

.PHONY: all filelist subdirs clean veryclean

all: libnix_main.a

filelist:

subdirs:

clean:
	-rm -f *.o *.err

veryclean: clean
	-rm -f libnix_main.a

libnix_main.a: $(OBJECTS) ../../sources/nix_main/Makefile
	-rm -f $@
	$(AR) -q $@ $(OBJECTS)
	$(RANLIB) $@
	echo "\$$$(V)" >>$@
