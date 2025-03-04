CC=gcc-14
CFLAGS=-c -Wall -I. -fpic -g -fbounds-check -Werror
LDFLAGS=-L.
LIBS=-lcrypto

OBJS=tester.o util.o mdadm.o

%.o:	%.c %.h
	$(CC) $(CFLAGS) $< -o $@

tester:	$(OBJS) jbod.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(OBJS) tester

