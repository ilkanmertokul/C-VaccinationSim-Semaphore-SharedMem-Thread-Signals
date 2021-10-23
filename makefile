TARGETS = program

CC=gcc

CFLAGS = -Wall -Werror -g 

all: clean $(TARGETS)

$(TARGETS):
	$(CC) $(CFLAGS) midterm.c nvc.c -o $@ -lpthread -lrt

clean:
	rm -f $(TARGETS)
