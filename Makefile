
CFLAGS += -g -I.
LDFLAGS += -g

MAIN_SOURCE = main.c slcan.c isotp.c util.c log_stdout.c
MAIN_OBJECTS = $(MAIN_SOURCE:.c=.o)
CLEAN_FILES += $(MAIN_OBJECTS)
TARGETS += main

TEST_SOURCE = slcan.c isotp.c util.c _test.c log_stdout.c
TEST_OBJECTS = $(TEST_SOURCE:.c=.o)
CLEAN_FILES += $(TEST_OBJECTS)
TARGETS += _test

CLEAN_FILES += $(TARGETS)

all: $(TARGETS)

main: $(MAIN_OBJECTS) slcan.h
	$(CC) -o $@ $(MAIN_OBJECTS)

_test: $(TEST_OBJECTS) slcan.h
	$(CC) -o $@ $(TEST_OBJECTS)

clean:
	$(RM) $(CLEAN_FILES)
