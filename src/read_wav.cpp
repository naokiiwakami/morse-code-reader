#include <libgen.h>
#include <math.h>
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "default_decoder.h"
#include "fft.h"
#include "morse_decoder.h"
#include "morse_reader.h"
#include "morse_timing_tracker.h"

#define SAMPLES_PER_FETCH 256
#define WINDOW_SIZE (SAMPLES_PER_FETCH * 2)

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
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <morse_audio_wav>\n", basename(argv[0]));
    exit(1);
  }
  auto sound_file_name = argv[1];

  SF_INFO sf_info;
  SNDFILE *sndfile;
  sndfile = sf_open(sound_file_name, SFM_READ, &sf_info);
  if (sndfile == nullptr) {
    fprintf(stderr, "No such file: %s\n", sound_file_name);
    exit(1);
  }

  fprintf(stderr, "file       = %s\n", sound_file_name);
  fprintf(stderr, "channels   = %d\n", sf_info.channels);
  fprintf(stderr, "samplerate = %d\n", sf_info.samplerate);
  fprintf(stderr, "channels   = %d\n", sf_info.channels);
  fprintf(stderr, "format     = 0x%x\n", sf_info.format);

  short buffers[2][SAMPLES_PER_FETCH];
  short *current_buffer;
  short *prev_buffer;
  prev_buffer = buffers[0];
  current_buffer = buffers[1];
  memset(prev_buffer, 0, sizeof(short));
  sf_count_t num_samples;
  auto *reader = new MorseReader(new DefaultMorseDecoder{}, SAMPLES_PER_FETCH);
  // auto *reader =
  //     new MorseReader(new StubMorseDecoder{false}, SAMPLES_PER_FETCH);
  reader->Verbose(false);

  bool first = true;
  do {
    num_samples = sf_read_short(sndfile, current_buffer, SAMPLES_PER_FETCH);
    if (!first) {
      reader->Read(prev_buffer, current_buffer, num_samples);
    }
    first = false;
    short *temp = prev_buffer;
    prev_buffer = current_buffer;
    current_buffer = temp;
  } while (num_samples == SAMPLES_PER_FETCH);
  printf("\n");

  delete reader;

  return 0;
}