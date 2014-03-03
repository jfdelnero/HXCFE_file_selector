#CC=i386-pc-linux-gcc
CC=m68k-amigaos-gcc.exe
CFLAGS= -O3 -I . -noixemul -I ./fat32
LDFLAGS=-s  -Wl,-Map,foo.map   -noixemul  -amiga-debug-hunk
EXEC=HXCFEMNG

all: $(EXEC)
	#m68k-atari-mint-strip -s $(EXEC)
	#./upx  -9 $(EXEC)
	mv $(EXEC) "D:\SDHxCFloppySelector.amigados"
	cmd /c 'startfe.bat' &

HXCFEMNG:      fectrl.o gui_utils.o amiga_hw.o crc.o fat_access.o fat_filelib.o fat_misc.o fat_string.o fat_table.o fat_write.o fat_cache.o
	$(CC) -o $@    $^ $(LDFLAGS)

fectrl.o: fectrl.c
	$(CC) -o $@ -c $< $(CFLAGS)

gui_utils.o: gui_utils.c
	$(CC) -o $@ -c $< $(CFLAGS)

amiga_hw.o: amiga_hw.c
	$(CC) -o $@ -c $< $(CFLAGS)

crc.o: crc.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_access.o: ./fat32/fat_access.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_filelib.o: ./fat32/fat_filelib.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_misc.o: ./fat32/fat_misc.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_string.o: ./fat32/fat_string.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_table.o: ./fat32/fat_table.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_write.o: ./fat32/fat_write.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_cache.o: ./fat32/fat_cache.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o


mrproper: clean
	rm -rf $(EXEC)

.PHONY: clean mrproper
