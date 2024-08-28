#include <libgen.h>
#include <math.h>
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pulse/error.h>
#include <pulse/simple.h>

#include "default_decoder.h"
#include "fft.h"
#include "morse_decoder.h"
#include "morse_reader.h"
#include "morse_timing_tracker.h"

#define BUFFER_SIZE 256
#define WINDOW_SIZE (BUFFER_SIZE * 2)

class StubMorseDecoder : public MorseDecoder {
private:
  bool silent_;

public:
  StubMorseDecoder(bool silent) : silent_(silent) {}

  void Dit() {
    if (!silent_) {
      printf("(dit)");
    }
  }

  void Dah() {
    if (!silent_) {
      printf("(dah)");
    }
  }

  void Break() {
    if (!silent_) {
      printf("( )");
    }
  }

  void Space() {
    if (!silent_) {
      printf("(/)");
    }
  }
};

int main(int argc, char *argv[]) {
  int iarg = 1;
  bool verbose = false;
  if (iarg < argc && strcmp(argv[iarg], "-v") == 0) {
    verbose = true;
    ++iarg;
  }
  if (iarg >= argc) {
    fprintf(stderr, "Usage: %s <morse_audio_wav>\n", basename(argv[0]));
    exit(1);
  }
  auto sound_file_name = argv[iarg];

  // setup input file
  SF_INFO sf_info = {0};
  SNDFILE *sndfile;
  sndfile = sf_open(sound_file_name, SFM_READ, &sf_info);
  if (sndfile == nullptr) {
    fprintf(stderr, "No such file: %s\n", sound_file_name);
    exit(1);
  }

  if (verbose) {
    fprintf(stderr, "file       = %s\n", sound_file_name);
    fprintf(stderr, "channels   = %d\n", sf_info.channels);
    fprintf(stderr, "samplerate = %d\n", sf_info.samplerate);
    fprintf(stderr, "channels   = %d\n", sf_info.channels);
    fprintf(stderr, "format     = 0x%x\n", sf_info.format);
  }

  printf("\n");

  // setup playback environment
  pa_sample_spec ss;
  memset(&ss, 0, sizeof(ss));
  ss.format = PA_SAMPLE_S16LE;
  ss.rate = sf_info.samplerate;
  ss.channels = sf_info.channels;

  int error;
  pa_simple *pa = pa_simple_new(NULL, argv[0], PA_STREAM_PLAYBACK, NULL,
                                "playback", &ss, NULL, NULL, &error);
  if (!pa) {
    fprintf(stderr, ": pa_simple_new() failed: %s\n", pa_strerror(error));
    sf_close(sndfile);
    return -1;
  }

  // setup buffers
  short buffers[2][BUFFER_SIZE];
  short *current_buffer;
  short *prev_buffer;
  prev_buffer = buffers[0];
  current_buffer = buffers[1];
  memset(prev_buffer, 0, sizeof(short));

  // setup the morse reader
  auto *morse_decoder = new DefaultMorseDecoder{};
  auto *morse_reader = new MorseReader(morse_decoder, BUFFER_SIZE);
  morse_reader->Verbose(false);

  // read and process data of approximately 6ms for each in the loop
  bool first = true;
  sf_count_t num_samples;
  do {
    num_samples = sf_read_short(sndfile, current_buffer, BUFFER_SIZE);

    if (pa_simple_write(pa, current_buffer,
                        (size_t)(num_samples * sizeof(*current_buffer)),
                        &error) < 0) {
      fprintf(stderr, __FILE__ ": pa_simple_write() failed: %s\n",
              pa_strerror(error));
      if (pa) {
        pa_simple_free(pa);
      }

      return -1;
    }

    if (!first) {
      morse_reader->Process(prev_buffer, current_buffer, num_samples);
    }
    first = false;
    short *temp = prev_buffer;
    prev_buffer = current_buffer;
    current_buffer = temp;

  } while (num_samples == BUFFER_SIZE);
  printf("\n\n");

  if (pa_simple_drain(pa, &error) < 0) {
    fprintf(stderr, __FILE__ ": pa_simple_drain() failed: %s\n",
            pa_strerror(error));
  }

  // shutdown
  if (pa) {
    pa_simple_free(pa);
  }
  delete morse_reader;
  sf_close(sndfile);

  return 0;
}