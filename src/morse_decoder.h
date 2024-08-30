#ifndef MORSE_DECODER_H_
#define MORSE_DECODER_H_

namespace morse {

class MorseDecoder {
public:
  virtual ~MorseDecoder() {}
  virtual void Dit() = 0;
  virtual void Dah() = 0;
  virtual void Break() = 0;
  virtual void Space() = 0;
};

} // namespace morse

#endif // MORSE_DECODER_H_
