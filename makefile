TARGETS = program

CC=gcc

CFLAGS = -Wall -Werror -g 

all: clean $(TARGETS)

$(TARGETS):
	$(CC) $(CFLAGS) main.c nvc.c -o $@ -lpthread -lrt

clean:
	rm -f $(TARGETS)
