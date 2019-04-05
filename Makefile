CFLAGS += -I.

_test: slcan.o _test.o
	$(CC) -o $@ $^

clean:
	$(RM) slcan.o _test slcan

