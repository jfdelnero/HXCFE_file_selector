#CC=i386-pc-linux-gcc
CC=m68k-amigaos-gcc
AS=m68k-amigaos-as

CFLAGS= -Os -I . -I .. -nostdlib -I ../fat32 -DFATFS_IS_LITTLE_ENDIAN=0 -m68000 -Wall -msoft-float -fno-builtin -fomit-frame-pointer #-DDEBUG
LDFLAGS=-s  -Wl,-Map,foo.map -nostdlib -m68000

EXEC=HXCFEMNG

all: $(EXEC) bootblock.bin
	m68k-amigaos-strip -s $(EXEC)
	#./upx  -9 $(EXEC)
	#./Shrinkler $(EXEC) $(EXEC).PACK
	#mv $(EXEC).PACK "H:\SDHxCFloppySelector.amigados\$(EXEC)"
	mv $(EXEC) "H:\SDHxCFloppySelector.amigados\$(EXEC)"
	cmd /c 'startfe.bat' &

HXCFEMNG: libc/ncrt0.o libc/amiga_api.o libc/mStartupAsmC.o libc/mStartup.o libc/snprintf/snprintf.o libc/ncrt0.o fectrl.o gui_utils.o amiga.o crc.o fat_access.o fat_filelib.o fat_misc.o fat_string.o fat_table.o fat_write.o fat_cache.o reboot.o msg_txt.o
	$(CC) -o $@    $^ $(LDFLAGS)

reboot.o: reboot.S
	$(AS) -o $@ $<

fectrl.o: ../fectrl.c
	$(CC) -o $@ -c $< $(CFLAGS)

gui_utils.o: ../gui_utils.c
	$(CC) -o $@ -c $< $(CFLAGS)

msg_txt.o: ../msg_txt.c
	$(CC) -o $@ -c $< $(CFLAGS)

amiga.o: amiga.c
	$(CC) -o $@ -c $< $(CFLAGS)

crc.o: crc.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_access.o: ../fat32/fat_access.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_filelib.o: ../fat32/fat_filelib.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_misc.o: ../fat32/fat_misc.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_string.o: ../fat32/fat_string.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_table.o: ../fat32/fat_table.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_write.o: ../fat32/fat_write.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_cache.o: ../fat32/fat_cache.c
	$(CC) -o $@ -c $< $(CFLAGS)

bootblock.bin: make_bootsector bootblock_code.bin
	./make_bootsector bootblock_code.bin bootblock.bin

bootblock_code.o: bootblock_code.S
	$(AS) -o $@ $< -m68000

bootblock_code.bin : bootblock_code.o
	m68k-amigaos-objcopy -O binary $< $@

make_bootsector: make_bootsector.c
	gcc   -o $@ $<

libc/mStartup.o: libc/mStartup.c
	$(CC) $(CFLAGS) -c $< -o $@

libc/snprintf/snprintf.o: libc/snprintf/snprintf.c libc/snprintf/snprintf.h
	$(CC) $(CFLAGS) -c $< -o $@

libc/mStartupAsmC.o: libc/mStartupAsmC.s
	$(AS) -o $@ $< -m68000

libc/amiga_api.o: libc/amiga_api.S
	$(AS) -o $@ $< -m68000

libc/ncrt0.o: libc/ncrt0.S
	$(AS) -o $@ $< -m68000

clean:
	rm -rf *.o *.bin make_bootsector.exe

mrproper: clean
	rm -rf $(EXEC)

.PHONY: clean mrproper
