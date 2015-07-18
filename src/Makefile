CFLAGS = -g -O2 -Wall
OBJS = slog.o

.c.o:
	$(CC) $(CFLAGS) -c $< $(LIB)

libslog.a: $(OBJS)
	$(AR) rcs libslog.a $(OBJS)
	@echo [-] Syncing static library
	sync

slog.o: slog.h 

.PHONY: clean
clean:
	$(RM) libslog.a $(OBJS)
