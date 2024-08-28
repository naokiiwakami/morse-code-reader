#include <math.h>
#include <stdio.h>
#include <string.h>

#include "morse_reader.h"

static const float kThreshold = 2.0e12;

MorseReader::MorseReader(MorseDecoder *decoder, size_t window_size)
    : window_size_(window_size) {
  window_ = new float[window_size_ * 2];
  MakeBlackmanNuttallWindow(window_size_ * 2, window_);
  input_data_ = new complex[window_size_ * 2];
  temp_data_ = new complex[window_size_ * 2];

  timing_tracker_ = new MorseTimingTracker(decoder);
}

MorseReader::~MorseReader() {
  delete timing_tracker_;
  delete[] input_data_;
  delete[] temp_data_;
  delete[] window_;
}

MorseReader *MorseReader::Verbose(bool value) {
  verbose_ = value;
  return this;
}

void MorseReader::Process(short prev_buffer[], short current_buffer[],
                          size_t current_buffer_size) {
  MakeInputData(input_data_, window_, prev_buffer, current_buffer,
                current_buffer_size);
  fft(input_data_, window_size_ * 2, temp_data_);

  float current = Power(input_data_[0]);
  int value = 0;
  for (size_t i = 0; i < window_size_; ++i) {
    float next = i < window_size_ - 1 ? Power(input_data_[i + 1]) : 0.0;
    if (current > kThreshold) {
      value = 1;
    }
    temp_data_[i].Re = value;
    current = next;
  }
  timing_tracker_->Proceed();
  if (value && !prev_value_) {
    timing_tracker_->Rise();
  }
  if (!value && prev_value_) {
    timing_tracker_->Fall();
  }
  if (verbose_) {
    printf("%c", value ? '^' : '_');
  }
  prev_value_ = value;
}

void MorseReader::MakeBlackmanNuttallWindow(size_t window_size,
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

float MorseReader::MakeInputData(complex input_data[], float window[],
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
