CC=gcc
AS=as
CFLAGS= -O0 -I . -I .. -I ../fat32 -g
LDFLAGS=-s  -Wl,-Map,foo.map -lSDL
EXEC=HXCFEMNG

all: $(EXEC)
	strip -s $(EXEC)

HXCFEMNG:      fectrl.o gui_utils.o sdl_hal.o fat_access.o fat_filelib.o fat_misc.o fat_string.o fat_table.o fat_write.o fat_cache.o msg_txt.o
	$(CC) -o $@    $^ $(LDFLAGS)

fectrl.o: ../fectrl.c
	$(CC) -o $@ -c $< $(CFLAGS)

gui_utils.o: ../gui_utils.c
	$(CC) -o $@ -c $< $(CFLAGS)

msg_txt.o: ../msg_txt.c
	$(CC) -o $@ -c $< $(CFLAGS)

sdl_hal.o: sdl_hal.c
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

clean:
	rm -rf *.o


mrproper: clean
	rm -rf $(EXEC)

.PHONY: clean mrproper