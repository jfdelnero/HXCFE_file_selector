# Generated automatically from Makefile.in by configure.
# make startup-files
# 13-Apr-94 G. Nikl
# 14-Apr-94 M. Fleischer clean+veryclean added

#### Start of system configuration section. ####

srcdir =	/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/sources/startup
VPATH =		/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/sources/startup

CC =		m68k-amigaos-gcc
CPP =		m68k-amigaos-gcc -E
AS =		m68k-amigaos-as

AR =		m68k-amigaos-ar
RANLIB =	m68k-amigaos-ranlib
AWK =		gawk

#### End system configuration section ####

OPTIONS=-I$(srcdir)/../headers -O -fomit-frame-pointer

%.o: %.c
	$(CC) $(OPTIONS) -c $^ 2>&1|tee $*.err
	-if test ! -s $*.err; then rm $*.err; fi
	echo "\$$$(V)" >>$@

.PHONY: all filelist subdirs clean veryclean

all: nrcrt0.o nbcrt0.o nlrcrt0.o nlbcrt0.o ncrt0.o libinit.o libinitr.o devinit.o

subdirs:

filelist:

clean:

veryclean:
	-rm *.o

# baserelative resident startup-code

nrcrt0.o: nrcrt0.S
	$(CC) -c $^
	echo "\$$$(V)" >>$@

# large baserelative resident startup-code

nlrcrt0.o: nlrcrt0.S
	$(CC) -m68020 -c $^
	echo "\$$$(V)" >>$@
# baserelative startup code

nbcrt0.o: nbcrt0.S
	$(CC) -c $^
	echo "\$$$(V)" >>$@

# large baserelative startup code

nlbcrt0.o: nlbcrt0.S
	$(CC) -m68020 -c $^
	echo "\$$$(V)" >>$@


# normal (=large) startup code

ncrt0.o: ncrt0.S
	$(CC) -c $^
	echo "\$$$(V)" >>$@
