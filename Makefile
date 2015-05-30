LIB = libslog/slog.a
CFLAGS = -g -O2 -Wall -Ilibslog

example: example.c $(OBJ)
	gcc $(CFLAGS) -o example example.c $(LIB) $(OBJ)

example.o: example.h

.PHONY: clean

clean:
	$(RM) example $(OBJ)
