
CFLAGS += -g -I.
LDFLAGS += -g

TEST_SOURCE = slcan.c _test.c
TEST_OBJECTS = $(TEST_SOURCE:.c=.o)

all: _test

_test: $(TEST_OBJECTS) slcan.h
	$(CC) -o $@ $(TEST_OBJECTS)

clean:
	$(RM) slcan.o _test slcan

