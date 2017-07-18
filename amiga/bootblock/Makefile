CC=m68k-amigaos-gcc
AS=m68k-amigaos-as

CFLAGS= -Os -I . -nostdlib -DFATFS_IS_LITTLE_ENDIAN=0 -m68000 -Wall -msoft-float -fno-builtin -fomit-frame-pointer -mpcrel #-DDEBUG
LDFLAGS=-s  -Wl,-Map,foo.map -nostdlib -m68000 -mpcrel

EXEC=AUTOBOOT.ADF

all: $(EXEC)

AUTOBOOT.ADF: make_bootblock bootblock_bin.raw
	./make_bootblock bootblock_bin.raw ../AUTOBOOT.ADF ../HXCFEMNG

bootblock_bin.raw : bootblock_code.exe
	m68k-amigaos-objcopy -O binary $< $@

bootblock_code.exe: ncrt0.o amiga_api.o bootblock.o
	$(CC) -o $@    $^ $(LDFLAGS)
	m68k-amigaos-strip -s $@

amiga_api.o: amiga_api.S
	$(AS) -o $@ $< -m68000

ncrt0.o: ncrt0.S
	$(AS) -o $@ $< -m68000

bootblock.o : bootblock.c
	$(CC) $(CFLAGS) -c $< -o $@

make_bootblock: make_bootblock.c
	gcc   -o $@ $<

clean:
	rm -rf *.o *.bin *.raw *.exe *.ADF *.map 

mrproper: clean
	rm -rf $(EXEC)

.PHONY: clean mrproper
