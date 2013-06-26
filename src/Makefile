#
# Makefile for ART library (libart.a)
#

INC=	../inc
LIB=	../lib

LIBART=	$(LIB)/libart.a
INCS=	$(INC)/art.h \
		$(INC)/object.h \
		$(INC)/pair.h \
		$(INC)/pattern.h \
		$(INC)/json.h \
		$(INC)/actor.h
OBJS=	object.o \
		pair.o \
		pattern.o \
		json.o \
		actor.o

CFLAGS=	-I$(INC)
#CFLAGS=	-ansi -pedantic -Wall -Wextra -ffreestanding -fno-stack-protector -I$(INC)

all: $(LIBART) art

test: all
	./art

$(LIBART): $(OBJS)
	$(AR) rv $@ $(OBJS)

$(OBJS): $(INCS)

art: art.o $(LIBART)
	$(CC) $(CFLAGS) -o $@ art.o $(LIBART)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean : 
	rm -rf $(LIB)
	mkdir $(LIB)
	rm -f art
	rm -f *.o *~
