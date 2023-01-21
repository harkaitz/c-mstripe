DESTDIR  =
PREFIX   =/usr/local
CC       =cc
CFLAGS   =-Wall -g
LIBS     =-lcurl -ljansson
PROGRAMS =\
    prog/mstripe-customer \
    prog/mstripe-language
HEADERS  =mstripe.h mstripe-customer.h
## -- targets
all: $(PROGRAMS)
clean:
	rm -f $(PROGRAMS)
install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(PROGRAMS) $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/include
	cp $(HEADERS) $(DESTDIR)$(PREFIX)/include
## -- programs
prog/%: prog/%.c $(HEADERS)
	$(CC) -o $@ $< $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LIBS)
## -- license --
install: install-license
install-license: LICENSE
	mkdir -p $(DESTDIR)$(PREFIX)/share/doc/c-mstripe
	cp LICENSE $(DESTDIR)$(PREFIX)/share/doc/c-mstripe
## -- license --
