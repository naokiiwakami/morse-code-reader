#ifndef MORSE_TIMING_TRACKER_H_
#define MORSE_TIMING_TRACKER_H_

#include <cstdint>

class MorseDecoder {
public:
  virtual ~MorseDecoder() {}
  virtual void Dit() = 0;
  virtual void Dah() = 0;
  virtual void Break() = 0;
  virtual void Space() = 0;
};

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