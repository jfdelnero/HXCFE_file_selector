# Generated automatically from Makefile.in by configure.
#### Start of system configuration section. ####

srcdir =	/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/examples
VPATH =		/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/examples

CC =		m68k-amigaos-gcc

#### End system configuration section ####

# Some examples

.PHONY: all

all: lib libr dev helloworld

# normal library
lib: libinit.o simplelib.c
	$(CC) -nostdlib -O3 -fomit-frame-pointer -fbaserel $^ -o $@

# multibase library
libr: libinitr.o simplelib.c
	$(CC) -nostdlib -O3 -fomit-frame-pointer -resident $^ -o $@

# device
dev: devinit.o simpledev.c
	$(CC) -nostdlib -O3 -fomit-frame-pointer -fbaserel $^ -o $@

# helloworld
helloworld: nbcrt0.o helloworld.c libstubs.a
	$(CC) -nostdlib -O3 -fomit-frame-pointer -s -fbaserel $^ -o $@
