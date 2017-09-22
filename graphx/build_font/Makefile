#CC=i386-pc-linux-gcc
CC=gcc

CFLAGS=-O3 -I . -g

LDFLAGS=-lc -lm  -ldl  -lpthread -g
EXEC=build_font

all: $(EXEC)
build_font: bmp_file.o main.o
	$(CC) -o $@    $^ $(LDFLAGS)

main.o: main.c
	$(CC) -o $@ -c $< $(CFLAGS)

bmp_file.o: bmp_file.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)

.PHONY: clean mrproper
