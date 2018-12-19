CC=gcc
CCFLAGS=-march=i686 -m32 -std=c99 -O1

RTE_SRC=ARRAY.s IO.s

RTE_OBJ=$(patsubst %.s,%.o,$(RTE_SRC))

.PHONY: clean test
.DEFAULT: test

%.o: %.s
	$(CC) $(CCFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

rte: $(RTE_OBJ)

snupl.o: snupl.c snupl.h
	$(CC) $(CCFLAGS) -c -o $@ $<

test: test_array test_io

test_array: test_array.c snupl.o $(RTE_OBJ)
	$(CC) $(CCFLAGS) -o $@ $^

test_io: test_io.c snupl.o $(RTE_OBJ)
	$(CC) $(CCFLAGS) -o $@ $^

clean:
	rm -rf *.o test_array test_io
