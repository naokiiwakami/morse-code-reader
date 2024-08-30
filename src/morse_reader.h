#ifndef MORSE_READER_H_
#define MORSE_READER_H_

#include <cstdint>
#include <vector>

#include "morse_decoder.h"

namespace morse {

enum ReaderState {
  IDLE,
  HIGH,
  LOW,
  BREAK,
};

class MorseReader {
private:
  MorseDecoder *decoder_;
  uint32_t clock_;
  ReaderState state_;
  int32_t last_interval_;
  uint32_t estimated_dit_length_;
  uint32_t dit_count_;
  float sum_dit_length_;

public:
  MorseReader(MorseDecoder *decoder);
  virtual ~MorseReader();
  void Proceed();
  void Rise();
  void Fall();
};

} // namespace morse

#endif // MORSE_READER_H_