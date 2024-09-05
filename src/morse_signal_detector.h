#ifndef MORSE_SIGNAL_DETECTOR_H_
#define MORSE_SIGNAL_DETECTOR_H_

#include <stdio.h>

#include <string>

#include <stddef.h>

#include "fft.h"
#include "monitor.h"
#include "morse_reader.h"

namespace morse {

class MorseSignalDetector {
private:
  size_t window_size_;
  float *window_;
  complex *input_data_;
  complex *temp_data_;

  uint8_t prev_value_ = 0;

  MorseReader *morse_reader_;

  bool verbose_ = false;
  FILE *dump_file_ = nullptr;
  FILE *analysis_file_ = nullptr;

  size_t window_count_ = 0;

public:
  MorseSignalDetector(MorseReader *morse_reader, size_t window_size);
  virtual ~MorseSignalDetector();

  void Verbose(bool value = true);

  int SetDumpFile(const std::string &pattern_file_name);

  int SetAnalysisFile(const std::string &analysis_file_name);

  void Process(short prev_buffer[], short current_buffer[],
               size_t current_buffer_size, Monitor *monitor);

private:
  void MakeBlackmanNuttallWindow(size_t window_size, float window[]);

  inline float Power(complex data) {
    return data.Re * data.Re + data.Im * data.Im;
  }

  float MakeInputData(complex input_data[], float window[], short prev[],
                      short current[], int n);
};

} // namespace morse

#endif // MORSE_SIGNAL_DETECTOR_H_
