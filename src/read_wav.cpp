#include <libgen.h>
#include <math.h>
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "default_decoder.h"
#include "fft.h"
#include "morse_reader.h"

#define SAMPLES_PER_FETCH 256
#define WINDOW_SIZE (SAMPLES_PER_FETCH * 2)

class StubMorseDecoder : public MorseDecoder {
public:
  void Dit() { printf("."); }

  void Dah() { printf("-"); }

  void Break() { printf(" "); }

  void Space() { printf("/"); }
};

void MakeBlackmanNuttallWindow(int window_size, float window[]) {
  float a0 = 0.3636819;
  float a1 = 0.4891775;
  float a2 = 0.1365995;
  float a3 = 0.0106411;
  for (int n = 0; n < window_size; ++n) {
    window[n] = a0 - a1 * cos((2 * M_PI * (n + 0.5)) / window_size) +
                a2 * cos((4 * M_PI * (n + 0.5)) / window_size) -
                a3 * cos((6 * M_PI * (n + 0.5)) / window_size);
  }
}

float Power(complex data) { return data.Re * data.Re + data.Im * data.Im; }

float MakeInputData(complex input_data[], float window[], short prev[],
                    short current[], int n) {
  float total_power = 0.0;
  for (int i = 0; i < n; ++i) {
    input_data[i].Re = window[i] * prev[i];
    input_data[i].Im = 0;
    total_power += Power(input_data[i]);
    input_data[i + n].Re = window[i + n] * current[i];
    input_data[i + n].Im = 0;
    total_power += Power(input_data[i + n]);
  }
  return total_power;
}

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

  float window[WINDOW_SIZE];
  MakeBlackmanNuttallWindow(WINDOW_SIZE, window);

  short buffers[2][SAMPLES_PER_FETCH];
  short *current_buffer;
  short *prev_buffer;
  prev_buffer = buffers[0];
  current_buffer = buffers[1];
  memset(prev_buffer, 0, sizeof(short));
  sf_count_t num_samples;
  int i_win = 0;
  float threshold = 3.0e12;
  int prev_value = 0;

  auto reader = new MorseReader(new DefaultMorseDecoder{});

  do {
    num_samples = sf_read_short(sndfile, current_buffer, SAMPLES_PER_FETCH);
    complex input_data[WINDOW_SIZE];
    complex temp_data[WINDOW_SIZE];
    MakeInputData(input_data, window, prev_buffer, current_buffer,
                  SAMPLES_PER_FETCH);
    fft(input_data, WINDOW_SIZE, temp_data);
    short *temp = prev_buffer;
    prev_buffer = current_buffer;
    current_buffer = temp;
    float current = Power(input_data[0]);
    int limit = WINDOW_SIZE / 2;
    int value = 0;
    for (int i = 0; i < limit; ++i) {
      float next = i < limit - 1 ? Power(input_data[i + 1]) : 0.0;
      if (current > threshold) {
        value = 1;
      }
      temp_data[i].Re = value;
      current = next;
    }
    reader->Proceed();
    if (value && !prev_value) {
      reader->Rise();
    }
    if (!value && prev_value) {
      reader->Fall();
    }
    ++i_win;
    prev_value = value;
  } while (num_samples == SAMPLES_PER_FETCH);
  printf("\n");

  delete reader;

  return 0;
}