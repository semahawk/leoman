CC = clang
CFLAGS = -g -std=c99

OBJS = sar

all: $(OBJS)

sar: sar.c ../kernel/sar.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o

distclean: clean
	rm -f $(OBJS)

