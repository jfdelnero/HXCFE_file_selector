CC=gcc
AS=as

CFLAGS= -O3 -Wall -I . -I .. -I ../fat32 -DFATFS_IS_LITTLE_ENDIAN=1 -DSDLHOST -DCORTEX_FW_SUPPORT
#CFLAGS= -O0 -Wall -I . -I .. -I ../fat32 -g -DFATFS_IS_LITTLE_ENDIAN=1 -DDEBUG -DSDLHOST #-fsanitize=address  -fno-omit-frame-pointer

LDFLAGS= -s -Wl,-Map,foo.map -lSDL
#LDFLAGS=  -Wl,-Map,foo.map  -lSDL -g #-lasan

EXEC=HXCFEMNG

all: $(EXEC)
	#strip -s $(EXEC)

HXCFEMNG:      fectrl.o errors_def.o media_access.o config_file.o menu.o menu_settings.o menu_selectdrive.o menu_commands.o gui_utils.o sdl_hal.o fat_access.o fat_filelib.o fat_misc.o fat_string.o fat_table.o fat_write.o fat_cache.o msg_txt.o slot_list_gen.o
	$(CC) -o $@    $^ $(LDFLAGS)

fectrl.o: ../fectrl.c
	$(CC) -o $@ -c $< $(CFLAGS)

errors_def.o: ../errors_def.c
	$(CC) -o $@ -c $< $(CFLAGS)

media_access.o: ../media_access.c
	$(CC) -o $@ -c $< $(CFLAGS)

config_file.o: ../config_file.c
	$(CC) -o $@ -c $< $(CFLAGS)

menu.o: ../menu.c
	$(CC) -o $@ -c $< $(CFLAGS)

menu_settings.o: ../menu_settings.c
	$(CC) -o $@ -c $< $(CFLAGS)

menu_selectdrive.o: ../menu_selectdrive.c
	$(CC) -o $@ -c $< $(CFLAGS)

menu_commands.o: ../menu_commands.c
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

slot_list_gen.o: slot_list_gen.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o


mrproper: clean
	rm -rf $(EXEC)

.PHONY: clean mrproper
