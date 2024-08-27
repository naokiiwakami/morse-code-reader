#ifndef MORSE_READER_H_
#define MORSE_READER_H_

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

class MorseReader {
private:
  MorseDecoder *decoder_;
  uint32_t clock_;
  ReaderState state_;
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

#endif // MORSE_READER_H_