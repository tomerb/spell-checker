CC = gcc
CFLAGS = -fPIC -Wall -Wextra -std=c99 -pedantic -g
LDFLAGS = -shared
RM = rm -f
TARGET_LIB = libspellcheck.so

SC_DEBUG = 0
ifeq (1,$(SC_DEBUG))
CFLAGS += -O0 -DDEBUG
else
CFLAGS += -O3
endif

SRCS = spell-checker.c spell-checker_runner.c spell-checker_data.c
OBJS = $(SRCS:.c=.o)

TEST_SRCS = spell-checker_test.c
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_LDFLAGS = -L. -lspellcheck -lpthread
TEST_TARGET_EXE = test_spellcheck

.PHONY: all
all: ${TARGET_LIB}

test: ${TARGET_LIB}
	$(CC) ${TEST_LDFLAGS} $(TEST_SRCS) -o ${TEST_TARGET_EXE}

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c):%.c
	$(CC) $(CFLAGS) $< >$@

$(TEST_SRCS:.c):%.c
	$(CC) $(CFLAGS) $< >$@

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${OBJS} ${TEST_OBJS} ${TEST_TARGET_EXE} $(SRCS:.c)
