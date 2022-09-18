DESTDIR  =
PREFIX   =/usr/local
CC       =cc
CFLAGS   =-Wall -g
LIBS     =-lcurl -ljansson
PROGRAMS =\
    prog/mstripe-customer \
    prog/mstripe-language
HEADERS  =mstripe.h mstripe-customer.h

##
all: $(PROGRAMS)
clean:
	@echo 'RM $(PROGRAMS)'
	@rm -f $(PROGRAMS)
install: all
	@echo 'I  $(DESTDIR)$(PREFIX)/bin/$(PROGRAM)'
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp $(PROGRAMS) $(DESTDIR)$(PREFIX)/bin
	@echo 'I  $(DESTDIR)$(PREFIX)/include/$(HEADER)'
	@mkdir -p $(DESTDIR)$(PREFIX)/include
	@cp $(HEADERS) $(DESTDIR)$(PREFIX)/include

##
prog/%: prog/%.c $(HEADERS)
	@echo 'CC $< -> $@'
	@$(CC) -o $@ $< $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LIBS)
## -- license --
ifneq ($(PREFIX),)
install: install-license
install-license: LICENSE
	@echo 'I share/doc/c-mstripe/LICENSE'
	@mkdir -p $(DESTDIR)$(PREFIX)/share/doc/c-mstripe
	@cp LICENSE $(DESTDIR)$(PREFIX)/share/doc/c-mstripe
endif
## -- license --
