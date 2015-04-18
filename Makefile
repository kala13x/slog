CFLAGS = -g -O2 -Wall
OBJ = slog.o

example: example.c $(OBJ)
	gcc $(CFLAGS) -o example example.c $(OBJ)

slog.o: slog.h

.PHONY: clean

clean:
	$(RM) example $(OBJ)
