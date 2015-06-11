LIB = ../src/slog.a
CFLAGS = -g -O2 -Wall -I../src

example: example.c $(OBJ)
	gcc $(CFLAGS) -o example example.c $(LIB) $(OBJ)

example.o: example.h

.PHONY: clean

clean:
	$(RM) example $(OBJ)
