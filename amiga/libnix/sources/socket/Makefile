# Generated automatically from Makefile.in by configure.
#### Start of system configuration section. ####

srcdir =	/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/sources/socket
VPATH =		/cygdrive/g/cvs/projets/FloppyEmulator/Software/Floppy_Emulator_Control/Amiga/libnix/sources/socket

CC =		m68k-amigaos-gcc
CPP =		m68k-amigaos-gcc -E
AS =		m68k-amigaos-as

AR =		m68k-amigaos-ar
RANLIB =	m68k-amigaos-ranlib
AWK =		gawk

#### End system configuration section ####

ifneq ($(TARGET),clean)
include ../../sources/socket/filelist
endif

CURDIR = $(shell pwd)

OPTIONS=-I$(srcdir)/../headers -D_SOCKET_IO $(CFLAGS)

OBJECTS2=$(OBJECTS) \
         ../nix/assert/__eprintf.o \
         ../nix/extra/creat.o ../nix/extra/getw.o ../nix/extra/putw.o \
         ../nix/extra/setbuffer.o ../nix/extra/setlinebuf.o \
         ../nix/signal/raise.o ../nix/signal/signal.o ../nix/signal/__chkabort.o \
         ../nix/stdio/fdopen.o ../nix/stdio/fflush.o ../nix/stdio/fgetc.o \
         ../nix/stdio/fgetpos.o ../nix/stdio/fgets.o ../nix/stdio/fopen.o \
         ../nix/stdio/fprintf.o ../nix/stdio/fputc.o ../nix/stdio/fputs.o \
         ../nix/stdio/fread.o ../nix/stdio/freopen.o ../nix/stdio/fscanf.o \
         ../nix/stdio/fseek.o ../nix/stdio/fsetpos.o ../nix/stdio/ftell.o \
         ../nix/stdio/fwrite.o ../nix/stdio/gets.o ../nix/stdio/perror.o \
         ../nix/stdio/printf.o ../nix/stdio/puts.o ../nix/stdio/rewind.o \
         ../nix/stdio/scanf.o ../nix/stdio/setbuf.o ../nix/stdio/setvbuf.o \
         ../nix/stdio/snprintf.o ../nix/stdio/sprintf.o ../nix/stdio/sscanf.o \
         ../nix/stdio/tmpfile.o ../nix/stdio/tmpnam.o ../nix/stdio/ungetc.o \
         ../nix/stdio/vfprintf.o ../nix/stdio/vfscanf.o ../nix/stdio/vprintf.o \
         ../nix/stdio/vscanf.o ../nix/stdio/vsnprintf.o ../nix/stdio/vsprintf.o \
         ../nix/stdio/vsscanf.o ../nix/stdio/__fflush.o ../nix/stdio/__srget.o \
         ../nix/stdio/__stdio.o ../nix/stdio/__swbuf.o \
         ../nix/stdlib/abort.o

SUBDIRS2=$(SUBDIRS) \
         ../nix/assert ../nix/extra ../nix/signal ../nix/stdio ../nix/stdlib

REDEF=-D_DOSBase=___DOSBase

%.o: %.c
	$(CC) $(OPTIONS) -S $^ -o $*.S 2>&1|tee $*.err
	$(CPP) -traditional $(REDEF) $*.S -o $*2.S
	$(CC) $(OPTIONS) $*2.S -c -o $*.o
	-rm $*.S $*2.S
	-if test ! -s $*.err; then rm $*.err; fi

.PHONY: all clean veryclean

all: libsocket.a

clean:
	-rm -rf $(SUBDIRS2)

veryclean:
	-rm -rf *

$(foreach f,$(SUBDIRS2),$(CURDIR)/$(f)):
	mkdir $@

libsocket.a: $(foreach f,$(SUBDIRS2),$(CURDIR)/$(f)) $(OBJECTS2) ../../sources/socket/Makefile ../../sources/socket/filelist
	-rm -f $@
	$(AR) -q $@ $(OBJECTS2)
	$(RANLIB) $@
	echo "\$$$(V)" >>$@
