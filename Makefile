VERSION=0.0.1
MYCFLAGS=-Wall -fpic -fpie $(CFLAGS)
MYLDFLAGS=-pie -s -Wl,--export-dynamic $(LDFLAGS)
DESTDIR=

LIBS=-lOpenSLES

all: android-audio-sink

android-audio-sink: main.c 
	$(CC) $(MYCFLAGS) $(MYLDFLAGS) -DVERSION=\"$(VERSION)\" -o android-audio-sink $(INCLUDES) $(LIBS) -DVERSION=\"$(VERSION)\" main.c $(EXTRA_OBJS)

clean:
	rm -rf android-audio-sink *.o 
