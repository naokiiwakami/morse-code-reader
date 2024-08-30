#ifndef MORSE_SIGNAL_DETECTOR_H_
#define MORSE_SIGNAL_DETECTOR_H_

#include <stdio.h>

#include <string>

#include <stddef.h>

#include "fft.h"
#include "morse_decoder.h"
#include "morse_reader.h"

class MorseSignalDetector {
private:
  size_t window_size_;
  float *window_;
  complex *input_data_;
  complex *temp_data_;

  int prev_value_ = 0;

  MorseReader *timing_tracker_;

  bool verbose_ = false;
  FILE *dump_file_ = nullptr;

public:
  MorseSignalDetector(MorseReader *timing_tracker, size_t window_size);
  virtual ~MorseSignalDetector();

  void Verbose(bool value = true);

  int SetDumpFile(const std::string &pattern_file_name);

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

#endif // MORSE_SIGNAL_DETECTOR_H_
