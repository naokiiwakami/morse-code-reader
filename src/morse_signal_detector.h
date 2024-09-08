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
  size_t num_buffers_;
  size_t buffer_size_;
  float *window_;
  complex *input_data_;
  complex *temp_data_;

  MorseReader *morse_reader_;

  bool verbose_ = false;
  FILE *dump_file_ = nullptr;
  FILE *analysis_file_ = nullptr;

  // used for signal detection
  size_t center_frequency_;
  size_t window_count_ = 0;
  size_t last_toggled_ = 0; // used to avoid chattering
  static const size_t kLookBackWindowSize = 3;
  float filtered_values_[kLookBackWindowSize];
  float peak_; // used to scale values

  // the detector keeps the result for a while since it may be amended
  static const size_t kDetectionDelay = 32;
  uint8_t detected_signal_[kDetectionDelay];
  size_t signal_ptr_;

public:
  MorseSignalDetector(MorseReader *morse_reader, size_t num_buffers,
                      size_t buffer_size, size_t center_frequency);
  virtual ~MorseSignalDetector();

  void Verbose(bool value = true);

  int SetDumpFile(const std::string &pattern_file_name);

  int SetAnalysisFile(const std::string &analysis_file_name);

  void Process(short *buffers[], size_t current_buffer_size, Monitor *monitor);

  void Drain(Monitor *monitor);

private:
  void MakeBlackmanNuttallWindow(size_t window_size, float window[]);

  inline float Power(complex data) {
    return data.Re * data.Re + data.Im * data.Im;
  }

  float Filter(float data[], float coefficients[], size_t num_taps);

  float MakeInputData(complex input_data[], float window[], short *buffers[],
                      int n);
};

} // namespace morse

#endif // MORSE_SIGNAL_DETECTOR_H_
