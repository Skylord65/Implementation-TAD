CC=gcc
CFLAGS=-std=c99 -Wextra -Wall -Werror -pedantic 
LDFLAGS=


ECHO=@
ifeq ($(VERBOSE),1)
# VERBOSE : when compiling, echo the executed command line
	ECHO=
endif

EXEC=stack_ex1
SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)

all: 
	@$(MAKE) $(EXEC)

$(EXEC): $(OBJ)
	$(ECHO)$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(ECHO)$(CC) -o $@ -c $< $(CFLAGS)

staticstack.o : stack.h
main.o: stack.h

