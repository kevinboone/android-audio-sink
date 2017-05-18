VERSION=0.0.2
MYCFLAGS=-Wall -fpic -fpie $(CFLAGS)
MYLDFLAGS=-pie -s -Wl,--export-dynamic $(LDFLAGS)
DESTDIR=
PREFIX=/usr

LIBS=-lOpenSLES

all: android-audio-sink

android-audio-sink: main.c 
	$(CC) $(MYCFLAGS) $(MYLDFLAGS) -DVERSION=\"$(VERSION)\" -o android-audio-sink $(INCLUDES) $(LIBS) -DVERSION=\"$(VERSION)\" main.c $(EXTRA_OBJS)

install:
	mkdir -p $(DESTDIR)/$(PREFIX)/bin/
	mkdir -p $(DESTDIR)/$(PREFIX)/share/man/man1/
	cp -p android-audio-sink $(DESTDIR)/$(PREFIX)/bin/
	cp man1/* $(DESTDIR)/$(PREFIX)/share/man/man1/

clean:
	rm -rf android-audio-sink *.o 
