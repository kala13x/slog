CFLAGS = -g -O2 -Wall -fPIC
OBJS = slog.o

.c.o:
	$(CC) $(CFLAGS) -c $< $(LIB)

slog.a: $(OBJS)
	$(AR) rcs slog.a $(OBJS)
	@echo [-] Syncing static library
	sync

slog.o: slog.h

.PHONY: clean
clean:
	$(RM) slog.a $(OBJS)
