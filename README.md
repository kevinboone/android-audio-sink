# android-audio-sink

This is a simple utility for use with Android native-code applications, that
takes uncompressed (e.g., WAV) audio data on a named pipe (or standard input), 
and plays
it using the Android OpenSLES API. Android provides no method for native code
to play audio using the 'ordinary' ALSA audio devices -- although present,
they are not
accessible to unprivileged apps.

android-audio-sink is designd to be run as a helper
by native-code applications that produce audio output. The rationale is to
make it easier to port audio applications to Android, without needing to
add a heap of Android-specific audio API calls to them.

The simplest use-case is one in which an application presents (or can be
made to present WAV audio directly to stdout, so:

$ my-audio-app - | android-audio-sink

In practice, the audio-producing app would probably open a named pipe,
and android-audio-sink would consume data from that pipe.

This utility has been seen to work on native Android, and with Android
emulation in ChromeOS. Naturally, you'll need to build it using the 
appropriate C compiler for your desired Android platform. For example:

    CC=/path/to/android/toolchain/bin/arm-linux-gcc make

The compiler needs to have the OpenSLES headers and libraries available
which, in practice, probably means compiling using the Android toolchain
provided by Google.

The alternative to producing audio this way is, of course, to root the
Android device and just access the ALSA audio devices directly. 
This utility is specifically designed to work

For more information, see the included man page.

