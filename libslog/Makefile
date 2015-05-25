CFLAGS = -g -O2 -Wall -fPIC
OBJS = slog.o

all: $(OBJS)
	$(CC) -shared -o slog.so $(LIB) $(OBJS)

slog.o: slog.h

.PHONY: clean
clean:
	$(RM) slog.so $(OBJS)
