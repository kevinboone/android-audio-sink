/*======================================================================
android-audio-sink

A simple wav/pcm player for OpenSLES on Android with KBOX.
This utility reads a wav or pcm stream on stdin, and plays it
until end-of-file

If the input is a raw PCM stream, then command-line switches 
will probably be needed to set the sampling rate, number of
channels, and bits per sample

Note: at present, this utility will not play any kind of file with
big-endian byte ordering.

======================================================================*/
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>
#include <SLES/OpenSLES.h>
#include "SLES/OpenSLES_Android.h"

#ifndef BOOL
typedef int BOOL;
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay = NULL;
static SLObjectItf outputMixObject = NULL;
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine = NULL;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = NULL;
static BOOL playing = FALSE;

static int bits_per_sample = 16;
static int channels = 2;
static int sample_rate = 44100;
static int fd = 0; // stdin by default

#define BUFFSIZ 16384 
static unsigned char buff [BUFFSIZ];
//
// Horrible kludge for Chromebooks -- some Chrome library ties to 
//  call this function, even though (presumably) no cryptography
//  is actually done. So somebody needs to provide the symbol. 
void CRYPTO_library_init(void){}


/*======================================================================
   show_usage 
=======================================================================*/

void show_usage (const char *argv0)
  {
  //__modsi3(1); // Why was this here?
  printf ("Usage: audio_app | %s [options] \n", argv0);
  printf (" -b {8|1}      Bits per sample\n");
  printf (" -c {1|2}      Number of channels\n");
  printf (" -f {file}     File or pipe to consume (default stdin)\n");
  printf (" -s {Hz}       Samples per second\n");
  printf (" -h            Show this message\n");
  printf (" -v            Show version\n");
  }


/*======================================================================
   bqPlayerCallback 
=======================================================================*/
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
assert(bq == bqPlayerBufferQueue);
assert(NULL == context);

SLresult result;
memset (buff, 0, sizeof (buff));
if (read (fd, buff, BUFFSIZ) > 0)
  {
//int i;
//for (i = 0; i < sizeof (buff); i += 2)
   {
   //char temp = buff[i];
   //buff[i] = buff[1 + 1];
   //buff[i+1] = temp;
   }
  result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, 
    buff, BUFFSIZ);
  assert(SL_RESULT_SUCCESS == result);
  }
else
  playing = FALSE;
}

/*======================================================================
 *   main
======================================================================*/
int main(int argc, char **argv)
{
int c;
BOOL version = FALSE, help = FALSE;

while ((c = getopt (argc, argv, "vhs:b:c:f:")) != -1)
  {
  switch (c)
    {
    case 'v': 
      version = TRUE; break;
    case 'h': 
      help = TRUE; break;
    case 's': 
      sample_rate = atoi (optarg);
      if (sample_rate == 0)
       {
       fprintf (stderr, "%s: invalid sample rate", argv[0]);
       exit (-1);
       }
      break;
    case 'c': 
      channels = atoi (optarg);
      if (channels == 0)
       {
       fprintf (stderr, "%s: invalid number of channels", argv[0]);
       exit (-1);
       }
      break;
    case 'b': 
      bits_per_sample = atoi (optarg);
      if (bits_per_sample == 0)
       {
       fprintf (stderr, "%s: invalid bits per sample", argv[0]);
       exit (-1);
       }
      break;
    case 'f': 
      fd = open (optarg, O_RDONLY);
      if (fd < 0)
       {
       fprintf (stderr, "%s: can't open: ", optarg);
       exit (-1);
       }
      break;
    }
  }

if (version)
  {
  printf ("android-audio-sink v%s\n", VERSION);
  printf ("(c)2013-2017 Kevin Boone\n");
  exit (0);
  }

if (help)
  {
  show_usage(argv[0]);
  exit (0);
  }

SLresult result = slCreateEngine (&engineObject, 0, NULL, 0, NULL, NULL);
assert (result == SL_RESULT_SUCCESS);

result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
assert (result == SL_RESULT_SUCCESS);

result = (*engineObject)->GetInterface(engineObject, 
  SL_IID_ENGINE, &engineEngine);
assert(SL_RESULT_SUCCESS == result);

result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 
  0, NULL, NULL);
assert(SL_RESULT_SUCCESS == result);

result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
assert(SL_RESULT_SUCCESS == result);

// Read the stream, because we need to know how to set up the
//  buffer queue 
read (fd, buff, BUFFSIZ); 

if (buff[0] == 'R' && buff[1] == 'I' && buff[2] == 'F' && buff[3] == 'F')
  {
  // It's a WAV
  channels = (int) buff[22];
  sample_rate = buff [25] * 256 + buff[24]; 
  bits_per_sample = buff[34];

  //printf ("channels = %d, samp = %d, bits=%d\n", channels, sample_rate,
  //  bits_per_sample);
  memset (buff, 0, 44); // Blank off the RIFF header. Ugly, but it works.
  }
else
  {
  // Better hope user specified some parameters on the command line, because
  //  the defaults might not be correct
  }

SLDataLocator_AndroidSimpleBufferQueue loc_bufq = 
  {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, channels, 
  sample_rate * 1000,
  bits_per_sample, bits_per_sample, 
  (channels == 1 ? SL_SPEAKER_FRONT_CENTER : 
    SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT), 
  SL_BYTEORDER_LITTLEENDIAN};
SLDataSource audioSrc = {&loc_bufq, &format_pcm};

SLDataLocator_OutputMix loc_outmix = 
  {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};

SLDataSink audioSnk = {&loc_outmix, NULL};
const SLInterfaceID asids[2] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND};
const SLboolean asreq[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, 
  &audioSrc, &audioSnk, 2, asids, asreq);
if (SL_RESULT_SUCCESS == result)
  {
  result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
  assert(SL_RESULT_SUCCESS == result);

  result = (*bqPlayerObject)->GetInterface(bqPlayerObject, 
    SL_IID_PLAY, &bqPlayerPlay);
  assert(SL_RESULT_SUCCESS == result);

  result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
    &bqPlayerBufferQueue);
  assert(SL_RESULT_SUCCESS == result);

  result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, 
    bqPlayerCallback, NULL);
  assert(SL_RESULT_SUCCESS == result);

  result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, 
    buff, BUFFSIZ);
  assert(SL_RESULT_SUCCESS == result);

  result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
  assert(SL_RESULT_SUCCESS == result);

  playing = TRUE;
  do
    {
    usleep (500000);
    // This doesn't work -- GetPlayState can't be relied on to indicate
    //  whether we're still playing or not. Don't know why.
    //SLuint32 pState;
    //(*bqPlayerPlay)->GetPlayState(bqPlayerPlay, &pState);
    //if (pState != SL_PLAYSTATE_PLAYING) playing = FALSE;
    } while (playing);
  }
else
  {
  fprintf (stderr, "%s: Can't initialize player:\n"
    "channels=%d, sample_rate=%d, bits_per_sample=%d\n", argv[0],
    channels, sample_rate, bits_per_sample);
  }

if (bqPlayerObject)
  (*bqPlayerObject)->Destroy(bqPlayerObject);
bqPlayerObject = NULL;

if (outputMixObject != NULL) 
  (*outputMixObject)->Destroy(outputMixObject);
outputMixObject = NULL;

if (engineObject != NULL) 
  (*engineObject)->Destroy(engineObject);
engineObject = NULL;

if (fd > 0) close (fd);

return 0;
}

