CC=gcc
CFLAGS=-std=c99 -Wextra -Wall -Werror -pedantic 
LIBDIR=../Lib
LDFLAGS=-L$(LIBDIR) -llab2 -lm

ECHO = 
ifeq ($(VERBOSE),no)
	ECHO=@
endif

ifeq ($(DEBUG),no)
	CFLAGS += -O3 -DNDEBUG
	LDFLAGS +=
else
	CFLAGS += -g 
	LDFLAGS +=
endif

EXEC=evaluate
SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)

ifeq ($(DEBUG),no)
$(info Generating in release mode)
else
$(info Generating in debug mode)
endif

all: $(EXEC)

$(EXEC): $(LIBDIR)/liblab2.a $(OBJ) 
	$(ECHO)$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c  
	$(ECHO)$(CC) -o $@ -c $< $(CFLAGS) -I$(LIBDIR)

.PHONY: clean mrproper doc

clean:
	$(ECHO)rm -rf $(OBJ)

mrproper:
	$(ECHO)rm -rf $(OBJ) $(EXEC) 
	@$(MAKE) -C $(LIBDIR) mrproper

$(LIBDIR)/liblab2.a:
ifeq ($(VERBOSE),no)
	@$(MAKE) -C $(LIBDIR) >/dev/null
else
	@$(MAKE) -C $(LIBDIR)
endif

doc:
	@$(MAKE) -C $(LIBDIR) doc

main.o:  $(LIBDIR)/token.h $(LIBDIR)/queue.h $(LIBDIR)/stack.h
