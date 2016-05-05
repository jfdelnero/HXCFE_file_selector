# Generated automatically from Makefile.in by configure.
#### Start of system configuration section. ####

srcdir =	/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/sources/stack
VPATH =		/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/sources/stack

CC =		m68k-amigaos-gcc
CPP =		m68k-amigaos-gcc -E
AS =		m68k-amigaos-as

AR =		m68k-amigaos-ar
RANLIB =	m68k-amigaos-ranlib
AWK =		gawk

#### End system configuration section ####

ifneq ($(TARGET),clean)
include ../../sources/stack/filelist
endif

OPTIONS=-I$(srcdir)/../headers $(CFLAGS)

REDEF=-D_DOSBase=___DOSBase

%.o: %.c
	$(CC) $(OPTIONS) -S $^ -o $*.S 2>&1|tee $*.err
	$(CPP) -traditional $(REDEF) $*.S -o $*2.S
	$(CC) $(OPTIONS) $*2.S -c -o $*.o
	-rm $*.S $*2.S
	-if test ! -s $*.err; then rm $*.err; fi

.PHONY: all clean veryclean

all: libstack.a

clean:
	-rm -rf *.o

veryclean:
	-rm -rf *

libstack.a: $(OBJECTS) ../../sources/stack/Makefile ../../sources/stack/filelist
	-rm -f $@
	$(AR) -q $@ $(OBJECTS)
	$(RANLIB) $@
	echo "\$$$(V)" >>$@
