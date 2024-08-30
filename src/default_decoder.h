#ifndef DEFAULT_DECODER_H_
#define DEFAULT_DECODER_H_

#include <stdio.h>

#include <vector>

#include "event_listener.h"
#include "morse_decoder.h"

class DefaultMorseDecoder : public MorseDecoder {
private:
  int state_;

  std::vector<EventListener *> listeners_;

public:
  DefaultMorseDecoder();
  ~DefaultMorseDecoder() = default;

  void Subscribe(EventListener *listener);

  void Dit();
  void Dah();
  void Break();
  void Space();

private:
  int Decode(int state, int signal);
};

#endif // DEFAULT_DECODER_H_
