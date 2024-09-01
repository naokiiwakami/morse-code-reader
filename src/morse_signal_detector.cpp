#include "morse_signal_detector.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

namespace morse {

static const float kThreshold = 2.0e12;

MorseSignalDetector::MorseSignalDetector(MorseReader *timing_tracker,
                                         size_t window_size)
    : window_size_(window_size), morse_reader_(timing_tracker) {
  window_ = new float[window_size_ * 2];
  MakeBlackmanNuttallWindow(window_size_ * 2, window_);
  input_data_ = new complex[window_size_ * 2];
  temp_data_ = new complex[window_size_ * 2];
}

MorseSignalDetector::~MorseSignalDetector() {
  delete morse_reader_;
  delete[] input_data_;
  delete[] temp_data_;
  delete[] window_;
}

void MorseSignalDetector::Verbose(bool value) { verbose_ = value; }

int MorseSignalDetector::SetDumpFile(const std::string &pattern_file_name) {
  dump_file_ = fopen(pattern_file_name.c_str(), "w");
  return dump_file_ != nullptr ? 0 : -1;
}

void MorseSignalDetector::Process(short prev_buffer[], short current_buffer[],
                                  size_t current_buffer_size,
                                  Monitor *monitor) {
  MakeInputData(input_data_, window_, prev_buffer, current_buffer,
                current_buffer_size);
  fft(input_data_, window_size_ * 2, temp_data_);

  float current = Power(input_data_[0]);
  uint8_t value = 0;
  for (size_t i = 0; i < window_size_; ++i) {
    float next = i < window_size_ - 1 ? Power(input_data_[i + 1]) : 0.0;
    if (current > kThreshold) {
      value = 1;
    }
    temp_data_[i].Re = value;
    current = next;
  }

  monitor->AddSignal(value ? '^' : '_');

  if (dump_file_ == nullptr) {
    morse_reader_->Update(value);
    if (value != prev_value_) {
      monitor->Dump(morse_reader_);
    }
  } else {
    fprintf(dump_file_, "%c", value ? '^' : '_');
  }

  prev_value_ = value;
}

void MorseSignalDetector::MakeBlackmanNuttallWindow(size_t window_size,
                                                    float window[]) {
  float a0 = 0.3636819;
  float a1 = 0.4891775;
  float a2 = 0.1365995;
  float a3 = 0.0106411;
  for (size_t n = 0; n < window_size; ++n) {
    window[n] = a0 - a1 * cos((2 * M_PI * (n + 0.5)) / window_size) +
                a2 * cos((4 * M_PI * (n + 0.5)) / window_size) -
                a3 * cos((6 * M_PI * (n + 0.5)) / window_size);
  }
}

float MorseSignalDetector::MakeInputData(complex input_data[], float window[],
                                         short prev[], short current[], int n) {
  float total_power = 0.0;
  memset(current + n, 0, window_size_ - n);
  for (size_t i = 0; i < window_size_; ++i) {
    input_data[i].Re = window[i] * prev[i];
    input_data[i].Im = 0;
    total_power += Power(input_data[i]);
    input_data[i + n].Re = window[i + n] * current[i];
    input_data[i + n].Im = 0;
    total_power += Power(input_data[i + n]);
  }
  return total_power;
}

} // namespace morse