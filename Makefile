CC=gcc
CFLAGS=-c -g -Wall -std=c99 
·LDFLAGS=

SOURCES=my_shell.c nivel1.c nivel2.c nivel3.c nivel4.c nivel5.c nivel6.c
LIBRARIES=	#.o 
INCLUDES=	#.h
PROGRAMS=my_shell nivel1 nivel2 nivel3 nivel4 nivel5 nivel6

OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)

#$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
#	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

my_shell: my_shell.o 
	$(CC) $@.o -o $@ $(LIBRARIES)

nivel1: nivel1.o 
	$(CC) $@.o -o $@ $(LIBRARIES)
	
nivel2: nivel2.o  
	$(CC) $@.o -o $@ $(LIBRARIES)

nivel3: nivel3.o  
	$(CC) $@.o -o $@ $(LIBRARIES)

nivel4: nivel4.o  
	$(CC) $@.o -o $@ $(LIBRARIES)

nivel5: nivel5.o  
	$(CC) $@.o -o $@ $(LIBRARIES)

nivel6: nivel6.o  
	$(CC) $@.o -o $@ $(LIBRARIES)

%.o: %.c $(INCLUDES) 
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf *.o *~ *.tmp $(PROGRAMS)
