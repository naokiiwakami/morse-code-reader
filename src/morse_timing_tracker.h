#ifndef MORSE_TIMING_TRACKER_H_
#define MORSE_TIMING_TRACKER_H_

#include <cstdint>

#include "morse_decoder.h"

enum ReaderState {
  IDLE,
  HIGH,
  LOW,
  BREAK,
};

class MorseTimingTracker {
private:
  MorseDecoder *decoder_;
  uint32_t clock_;
  ReaderState state_;
  uint32_t estimated_dit_length_;
  uint32_t dit_count_;
  float sum_dit_length_;

public:
  MorseTimingTracker(MorseDecoder *decoder);
  virtual ~MorseTimingTracker();
  void Proceed();
  void Rise();
  void Fall();
};

#endif // MORSE_TIMING_TRACKER_H_