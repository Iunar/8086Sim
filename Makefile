.SILENT:
all: main.c
	gcc main.c -o decode86
	./decode86 listing_0038_many_register_mov
