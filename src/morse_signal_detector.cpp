#include "morse_signal_detector.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <utility>
#include <vector>

namespace morse {

static const float kThreshold = 2.0e12;

static float pprev_v = 0.0;
static float prev_v = 0.0;

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

int MorseSignalDetector::SetAnalysisFile(
    const std::string &analysis_file_name) {
  analysis_file_ = fopen(analysis_file_name.c_str(), "w");
  return analysis_file_ != nullptr ? 0 : -1;
}

static std::vector<size_t> Dispersion(const std::vector<double> &data,
                                      size_t lag, double influence,
                                      double threshold) {
  std::vector<size_t> peak_indices;
  std::vector<double> processed_data;
  size_t start_offset = 0;
  for (size_t i = 0; i < lag; ++i) {
    processed_data.push_back(data[i + start_offset]);
  }
  for (size_t index = lag; index < data.size(); ++index) {
    double avg = 0.0;
    for (size_t i = index - lag; i < index; ++i) {
      avg += processed_data[i];
    }
    avg /= lag;
    double stdev = 0.0;
    for (size_t i = index - lag; i < index; ++i) {
      stdev += (processed_data[i] - avg) * (processed_data[i] - avg);
    }
    stdev = sqrt(stdev / lag);
    double y = data[index + start_offset];
    double value = y - avg;
    if (value > stdev * threshold) {
      peak_indices.push_back(index);
      processed_data.push_back(influence * y +
                               (1 - influence) * processed_data[index - 1]);
    } else {
      processed_data.push_back(y);
    }
  }
  return peak_indices;
}

void MorseSignalDetector::Process(short prev_buffer[], short current_buffer[],
                                  size_t current_buffer_size,
                                  Monitor *monitor) {
  MakeInputData(input_data_, window_, prev_buffer, current_buffer,
                current_buffer_size);
  fft(input_data_, window_size_ * 2, temp_data_);

  const size_t kAnalysisSize = 100;
  std::vector<double> data{kAnalysisSize};
  for (size_t i = 0; i < kAnalysisSize; ++i) {
    data.push_back(Power(input_data_[i]));
  }

  uint8_t value = 0;

  // if (analysis_file_ != nullptr && window_count_ == 178 /* 4401 */) {
  auto peak_indices = Dispersion(data, 3, 0.2, 5);
  int center = 6;
  float v = data[center] - 0.4 * (data[center - 1] + data[center + 1]) -
            0.1 * (data[center - 2] + data[center + 2]);
  fprintf(analysis_file_, "%ld %f\n", window_count_,
          (v + prev_v + pprev_v) * 0.33);
  pprev_v = prev_v;
  prev_v = v;
  /*
  for (size_t i = 2; i <= 4; ++i) {
    fprintf(analysis_file_, "%ld %ld %f\n", window_count_, i, data[i]);
    // fprintf(analysis_file_, "%ld %f\n", i, data[i]);
  }
  */
#if 0
    int count = 0;
    for (size_t i = 0; i < peak_indices.size(); ++i) {
      auto index = peak_indices[i];
      // fprintf(analysis_file_, "%ld %f\n", index, data[index]);
      // /*
      fprintf(analysis_file_, "%ld %ld %f\n", window_count_, index,
              data[index]);
      // */
    }
    printf("%s", count > 0 ? "*" : "");
    fflush(stdout);
#endif
  // }
  ++window_count_;

  if (monitor != nullptr) {
    monitor->AddSignal(value ? '^' : '_');
  }

  if (dump_file_ == nullptr && analysis_file_ == nullptr) {
    morse_reader_->Update(value);
    if (value != prev_value_ && monitor != nullptr) {
      monitor->Dump(morse_reader_);
    }
  }

  if (dump_file_ != nullptr) {
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