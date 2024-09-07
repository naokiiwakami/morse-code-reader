#include "morse_signal_detector.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <utility>
#include <vector>

namespace morse {

static const float kThreshold = 2.0e12;

static float ppprev_v = 0.0;
static float pprev_v = 0.0;
static float prev_v = 0.0;

const size_t kDetectionDelay = 64;
static uint8_t values[kDetectionDelay];
static size_t values_ptr = 1;

MorseSignalDetector::MorseSignalDetector(MorseReader *timing_tracker,
                                         size_t num_buffers, size_t buffer_size)
    : num_buffers_(num_buffers), buffer_size_(buffer_size),
      morse_reader_(timing_tracker) {
  window_ = new float[buffer_size_ * num_buffers_];
  MakeBlackmanNuttallWindow(buffer_size_ * num_buffers_, window_);
  input_data_ = new complex[buffer_size_ * num_buffers_];
  temp_data_ = new complex[buffer_size_ * num_buffers_];
  memset(values, 0, sizeof(values));
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

static std::vector<size_t> DispersionOld(const std::vector<double> &data,
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

const size_t kLag = 5;
static double processed_data[kLag];
static size_t data_index = 0;
static float toggle = -5e10;

static int Dispersion(double y, double influence, double threshold) {
  if (data_index < kLag) {
    processed_data[data_index++] = y;
    return false;
  }
  double avg = 0.0;
  for (size_t i = 0; i < kLag; ++i) {
    avg += processed_data[i];
  }
  avg /= kLag;
  double stdev = 0.0;
  for (size_t i = 0; i < kLag; ++i) {
    stdev += (processed_data[i] - avg) * (processed_data[i] - avg);
  }
  stdev = sqrt(stdev / kLag);
  double value = y - avg;
  if (value > stdev * threshold) {
    processed_data[data_index % kLag] =
        influence * y +
        (1 - influence) * processed_data[(data_index - 1) % kLag];
    ++data_index;
    return 1;
  }
  processed_data[data_index++ % kLag] = y;
  return 0;
}

static size_t last_toggle_index = 0;
static float peak = 1.e11;

void MorseSignalDetector::Process(short *buffers[], size_t current_buffer_size,
                                  Monitor *monitor) {
  MakeInputData(input_data_, window_, buffers, current_buffer_size);
  memset(temp_data_, 0, sizeof(complex) * num_buffers_ * buffer_size_);
  fft(input_data_, buffer_size_ * num_buffers_, temp_data_);

  size_t center = 12;

  const size_t kAnalysisSize = 100;
  std::vector<double> data{kAnalysisSize};
  for (size_t i = 0; i < kAnalysisSize; ++i) {
    data.push_back(Power(input_data_[i]));
  }

  uint8_t value = 0;

  // if (analysis_file_ != nullptr && window_count_ == 178 /* 4401 */) {
  // auto peak_indices = DispersionOld(data, 3, 0.2, 5);
  /*
  float v = data[center] - 0.2 * data[center - 1] - 0.3 * data[center + 1] -
            0.3 * data[center - 2] - 0.2 * data[center + 2];
            */
  float v = data[center] - 0.3 * data[center - 1] - 0.3 * data[center + 1] -
            0.1 * data[center - 2] - 0.1 * data[center + 2];
  float current_value = (v + prev_v + pprev_v /*+ ppprev_v*/) * 0.33;
  // float current_intensity = (intensity + prev_intensity + pprev_intensity) *
  // 0.33;
  // bool on = current_value > 3.e10;
  // int on = Dispersion(current_value, 0.0125, 5);
  // bool on = !prev_on ? current_value > 3.e10 : current_value > 1.e10;
  // double diff = (prev_v - v);
  double diff = (ppprev_v + pprev_v - prev_v - v) * 0.5;
  value = prev_value_;
  if (value) {
    peak = std::max(peak, current_value);
  }
  if (window_count_ - last_toggle_index >= 15) {
    if (diff / peak < -1.3 && prev_value_) {
      size_t dot_length =
          static_cast<size_t>(morse_reader_->GetEstimatedDotLength());
      if (dot_length > 5) {
        for (size_t i = 0; i < dot_length; ++i) {
          values[(values_ptr - 1 - dot_length + i) % kDetectionDelay] = 0;
        }
      }
      toggle *= -1;
    } else if (!prev_value_) {
      if (current_value > 3.e10) {
        value = 1;
        peak = current_value;
      }
    } else if (current_value < peak / 20 || diff > 1.e11 /*||
               (current_value < peak / 5 && diff / peak > 0.3)*/) {
      value = 0;
    }
    if (value != prev_value_) {
      last_toggle_index = window_count_;
    }
  }
  values[values_ptr == 0 ? kDetectionDelay - 1 : values_ptr - 1] = value;
  /*
  float real = input_data_[center].Re * input_data_[center].Re +
               input_data_[center].Im * input_data_[center].Im;
               */
  if (analysis_file_ != nullptr) {
    fprintf(analysis_file_, "%ld %f %f %f %f\n", window_count_, current_value,
            values[values_ptr] ? 1.e11 : 0, diff, toggle);
  }
  ppprev_v = pprev_v;
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
    bool some_changed = morse_reader_->Update(values[values_ptr]);
    if (some_changed && monitor != nullptr) {
      monitor->Dump(morse_reader_);
    }
  }

  if (dump_file_ != nullptr) {
    fprintf(dump_file_, "%c", value ? '^' : '_');
  }

  prev_value_ = value;
  values_ptr = (values_ptr + 1) % kDetectionDelay;
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
                                         short *buffers[], int n) {
  float total_power = 0.0;
  memset(buffers[num_buffers_ - 1] + n, 0, buffer_size_ - n);
  for (size_t ibuf = 0; ibuf < num_buffers_; ++ibuf) {
    for (size_t i = 0; i < buffer_size_; ++i) {
      input_data[buffer_size_ * ibuf + i].Re =
          window[buffer_size_ * ibuf + i] * buffers[ibuf][i];
      input_data[buffer_size_ * ibuf + i].Im = 0;
      total_power += Power(input_data[i + buffer_size_ * ibuf]);
    }
  }
  return total_power;
}

} // namespace morse