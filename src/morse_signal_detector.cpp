#include "morse_signal_detector.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <utility>
#include <vector>

namespace morse {

static float kFreqDomainFilterCoef[] = {-0.1, -0.3, 1.0, -0.3, -0.1};
static const size_t kFreqDomainFilterSize =
    sizeof(kFreqDomainFilterCoef) / sizeof(*kFreqDomainFilterCoef);

static const float kThreshold = 2.0e12;

MorseSignalDetector::MorseSignalDetector(MorseReader *timing_tracker,
                                         size_t num_buffers, size_t buffer_size,
                                         size_t center_frequency)
    : num_buffers_(num_buffers), buffer_size_(buffer_size),
      morse_reader_(timing_tracker), center_frequency_(center_frequency) {
  window_ = new float[buffer_size_ * num_buffers_];
  MakeBlackmanNuttallWindow(buffer_size_ * num_buffers_, window_);
  input_data_ = new complex[buffer_size_ * num_buffers_];
  temp_data_ = new complex[buffer_size_ * num_buffers_];

  memset(filtered_values_, 0, sizeof(filtered_values_));
  peak_ = 1.e11;
  memset(detected_signal_, 0, sizeof(detected_signal_));
  signal_ptr_ = 1;
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

float MorseSignalDetector::Filter(float data[], float coefficients[],
                                  size_t num_taps) {
  float value = 0;
  for (size_t i = 0; i < num_taps; ++i) {
    value += data[i] * coefficients[i];
  }
  return value;
}

static ssize_t peak = -1;

void MorseSignalDetector::Process(short *buffers[], size_t current_buffer_size,
                                  Monitor *monitor) {
  MakeInputData(input_data_, window_, buffers, current_buffer_size);
  memset(temp_data_, 0, sizeof(complex) * num_buffers_ * buffer_size_);
  fft(input_data_, buffer_size_ * num_buffers_, temp_data_);

  size_t center = center_frequency_;

  const size_t kAnalysisSize = 100;
  std::vector<float> data{kAnalysisSize};
  for (size_t i = 0; i < kAnalysisSize; ++i) {
    data.push_back(Power(input_data_[i]));
  }

  float max_value = 0.0;
  std::vector<float> temp;
  for (size_t i = 0; i < kAnalysisSize - kFreqDomainFilterSize; ++i) {
    auto value =
        Filter(data.data() + i, kFreqDomainFilterCoef, kFreqDomainFilterSize);
    // fprintf(analysis_file_, "%ld %f\n", index, data[index]);
    temp.push_back(value);
    if (value > max_value && i >= 2 &&
        i < kAnalysisSize - kFreqDomainFilterSize - 2) {
      max_value = value;
      if (temp[i - 2] < value * 0.05) {
        peak = i + kFreqDomainFilterSize / 2;
      }
    }
  }
  move(0, 1);
  clrtoeol();
  printw("center: %ld", peak);
  for (int i = 0; i < 5; ++i) {
    move(i + 1, 1);
    clrtoeol();
    int level =
        (temp[4 * i] + temp[4 * i + 1] + temp[4 * i + 2] + temp[4 * i + 3]) *
        1.e-10;
    if (level > 50) {
      level = 50;
    }
    if (level < 0) {
      level = 0;
    }
    char buf[128];
    memset(buf, '*', level);
    buf[level] = 0;
    printw("%s", buf);
  }

  uint8_t current_signal = 0;

  // apply filter in frequency domain to retrieve peaks
  float v = Filter(data.data() + center - kFreqDomainFilterSize / 2,
                   kFreqDomainFilterCoef, kFreqDomainFilterSize);

  // apply filter in time domain to reduce noise
  float current_value = (v + filtered_values_[0] + filtered_values_[1]) * 0.33;
  // take value diff to detect rapid rise and drop
  float diff =
      (v + filtered_values_[0] - filtered_values_[1] - filtered_values_[2]) *
      0.5;

  // detect signal for this window
  uint8_t prev_signal = detected_signal_[(signal_ptr_ - 2) % kDetectionDelay];
  current_signal = prev_signal;
  if (current_signal) {
    peak_ = std::max(peak_, current_value);
  }
  if (window_count_ - last_toggled_ >= 2) {
    float change_factor = diff / peak_;
    if (change_factor > 1.3 && prev_signal) {
      // Special case of detecting steep rise while the signal is on.
      // It's likely the detector missed the previous drop due to noise in the
      // source. The detected signal would be amended.
      size_t dot_length =
          static_cast<size_t>(morse_reader_->GetEstimatedDotLength());
      if (dot_length > 5) {
        for (size_t i = 0; i < dot_length; ++i) {
          detected_signal_[(signal_ptr_ - 1 - dot_length + i) %
                           kDetectionDelay] = 0;
        }
        prev_signal = 0;
      }
    } else if (!prev_signal) {
      // TODO(Naoki): Is there a way to specify a relative value?
      if (current_value > 3.e10) {
        current_signal = 1;
        peak_ = current_value;
      }
    } else if (current_value < peak_ / 20 || change_factor < -1.3) {
      // turn off signal when the value becomes low enough or whena steep drop
      // is detected
      current_signal = 0;
    }

    if (current_signal != prev_signal) {
      last_toggled_ = window_count_;
    }
  }

  detected_signal_[signal_ptr_ == 0 ? kDetectionDelay - 1 : signal_ptr_ - 1] =
      current_signal;

  if (analysis_file_ != nullptr) {
    fprintf(analysis_file_, "%ld %f %f %f\n", window_count_, current_value,
            detected_signal_[signal_ptr_] ? 1.e11 : 0, diff);
    // current_signal ? 1.e11 : 0, diff);
    fflush(analysis_file_);
  }

#if 0
  int count = 0;
  for (size_t i = 0; i < kAnalysisSize - 10; ++i) {
    auto value =
        Filter(data.data() + i, kFreqDomainFilterCoef, kFreqDomainFilterSize);
    // fprintf(analysis_file_, "%ld %f\n", index, data[index]);
    // /*
    fprintf(analysis_file_, "%ld %ld %f\n", window_count_, i, value);
    // */
  }
  printf("%s", count > 0 ? "*" : "");
  fflush(stdout);
#endif
  ++window_count_;

  if (monitor != nullptr) {
    monitor->AddSignal(current_signal ? '^' : '_');
  }

  if (dump_file_ == nullptr && analysis_file_ == nullptr) {
    bool some_changed = morse_reader_->Update(detected_signal_[signal_ptr_]);
    if (some_changed && monitor != nullptr) {
      monitor->Dump(morse_reader_);
    }
  }

  if (dump_file_ != nullptr) {
    fprintf(dump_file_, "%c", current_signal ? '^' : '_');
  }

  for (int i = kLookBackWindowSize; --i >= 1;) {
    filtered_values_[i] = filtered_values_[i - 1];
  }
  filtered_values_[0] = v;
  signal_ptr_ = (signal_ptr_ + 1) % kDetectionDelay;
}

void MorseSignalDetector::Drain(Monitor *monitor) {
  if (dump_file_ == nullptr && analysis_file_ == nullptr) {

    for (size_t i = 0; i < kDetectionDelay; ++i) {
      bool some_changed = morse_reader_->Update(
          detected_signal_[(signal_ptr_ + i) % kDetectionDelay]);
      if (some_changed && monitor != nullptr) {
        monitor->Dump(morse_reader_);
      }
    }
  }
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