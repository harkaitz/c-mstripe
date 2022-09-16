DESTDIR =
PREFIX  =/usr/local
CC      =cc
CFLAGS  =-Wall -g
LIBS    =-lcurl -ljansson
PROGRAM =mstripe
HEADER  =mstripe.h

##
all: $(PROGRAM)
clean:
	@echo 'RM $(PROGRAM)'
	@rm -f $(PROGRAM)
install: all
	@echo 'I  $(DESTDIR)$(PREFIX)/bin/$(PROGRAM)'
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp $(PROGRAM) $(DESTDIR)$(PREFIX)/bin
	@echo 'I  $(DESTDIR)$(PREFIX)/include/$(HEADER)'
	@mkdir -p $(DESTDIR)$(PREFIX)/include
	@cp $(HEADER) $(DESTDIR)$(PREFIX)/include

##
$(PROGRAM): main.c mstripe.h
	@echo 'CC main.c -> $(PROGRAM)'
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
