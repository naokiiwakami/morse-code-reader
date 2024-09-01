#ifndef DEFAULT_DECODER_H_
#define DEFAULT_DECODER_H_

#include <stdio.h>

#include <vector>

#include "decoder.h"
#include "event_listener.h"

namespace morse {

class DefaultDecoder : public Decoder {
private:
  int state_;

  std::vector<EventListener *> listeners_;

public:
  DefaultDecoder();
  ~DefaultDecoder() = default;

  void Subscribe(EventListener *listener);

  void Dit();
  void Dah();
  void Break();
  void Space();

private:
  int Decode(int state, int signal);
};

} // namespace morse

#endif // DEFAULT_DECODER_H_
