LIB = -lslog -lpthread -lrt
CFLAGS = -g -O2 -Wall

example: example.c $(OBJ)
	gcc $(CFLAGS) -o example example.c $(LIB) $(OBJ)

example.o: example.h

.PHONY: clean

clean:
	$(RM) example *.log $(OBJ)
