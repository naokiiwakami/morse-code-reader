#ifndef DEFAULT_DECODER_H_
#define DEFAULT_DECODER_H_

#include <stdio.h>

#include "morse_timing_tracker.h"

class DefaultMorseDecoder : public MorseDecoder {
private:
  int state_;

public:
  DefaultMorseDecoder();
  ~DefaultMorseDecoder() = default;

  void Dit();
  void Dah();
  void Break();
  void Space();

private:
  int Decode(int state, int signal);
};

#endif // DEFAULT_DECODER_H_
