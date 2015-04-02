CXXFLAGS = -Wall -g
OBJ = slog.o

example: example.c $(OBJ)
	gcc $(CXXFLAGS) -o example example.c $(OBJ)

slog.o: slog.h

.PHONY: clean

clean:
	$(RM) example $(OBJ)