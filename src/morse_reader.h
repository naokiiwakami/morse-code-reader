#ifndef MORSE_READER_H_
#define MORSE_READER_H_

#include <stddef.h>

#include "fft.h"
#include "morse_decoder.h"
#include "morse_timing_tracker.h"

class MorseReader {
private:
  size_t window_size_;
  float *window_;
  complex *input_data_;
  complex *temp_data_;

  int prev_value_ = 0;

  MorseTimingTracker *timing_tracker_;

  bool verbose_ = false;

public:
  MorseReader(MorseDecoder *decoder, size_t window_size);
  virtual ~MorseReader();

  MorseReader *Verbose(bool value = true);

  void Process(short prev_buffer[], short current_buffer[],
               size_t current_buffer_size);

private:
  void MakeBlackmanNuttallWindow(size_t window_size, float window[]);

  inline float Power(complex data) {
    return data.Re * data.Re + data.Im * data.Im;
  }

  float MakeInputData(complex input_data[], float window[], short prev[],
                      short current[], int n);
};

#endif // MORSE_READER_H_
