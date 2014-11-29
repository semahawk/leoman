CC = clang
CFLAGS = -g -std=c99

OBJS = sar

all: $(OBJS)

sar: ../kernel/sar.h sar.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o

distclean: clean
	rm -f $(OBJS)

