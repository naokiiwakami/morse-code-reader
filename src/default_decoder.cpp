/*
 * Decoder implementation inspired by Chris Wellons
 * https://nullprogram.com/blog/2020/12/31/
 */
#include "default_decoder.h"

#include <string>

#define ERR 0x1

namespace morse {

DefaultDecoder::DefaultDecoder() : state_(0) {}

void DefaultDecoder::Subscribe(EventListener *listener) {
  listeners_.push_back(listener);
}

void DefaultDecoder::Dit() {
  for (auto listener : listeners_) {
    listener->OnEvent(EventType::SIGNAL, ".");
  }
  state_ = Decode(state_, '.');
}

void DefaultDecoder::Dah() {
  for (auto listener : listeners_) {
    listener->OnEvent(EventType::SIGNAL, "-");
  }
  state_ = Decode(state_, '-');
}

void DefaultDecoder::Break() {
  state_ = Decode(state_, 0);
  std::string out{state_ > 1 ? static_cast<char>(state_) : '?'};
  for (auto listener : listeners_) {
    listener->OnEvent(EventType::OUT, out);
  }
  state_ = 0;
}

void DefaultDecoder::Space() { printf(" "); }

int DefaultDecoder::Decode(int state, int c) {
  static const unsigned char table[] = {
      0x01, 0x45, 0x54, 0x49, 0x41, 0x4e, 0x4d, 0x53, 0x55, 0x52, 0x57, 0x44,
      0x4b, 0x47, 0x4f, 0x48, 0x56, 0x46, 0x01, 0x4c, 0x01, 0x50, 0x4a, 0x42,
      0x58, 0x43, 0x59, 0x5a, 0x51, 0x01, 0x01, 0x35, 0x34, 0x01, 0x33, 0x01,
      0x01, 0x01, 0x32, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x31, 0x36,
      0x3d, 0x2f, 0x01, 0x01, 0x01, 0x28, 0x01, 0x37, 0x01, 0x01, 0x01, 0x38,
      0x01, 0x39, 0x30, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x01, 0x01, 0x01, 0x3f, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x2e, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x27, 0x01, 0x01,
      0x2d, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x29, 0x01, 0x01, 0x01, 0x01, 0x01, 0x2c, 0x01, 0x01, 0x01, 0x01, 0x3a,
  };
  if (state > 0 || -state >= (int)(sizeof(table) / sizeof(*table))) {
    return ERR;
  }
  int value = table[-state];
  switch (c) {
  case '\0':
  case ' ':
    return value;
  case '.':
    return state * 2 - 1;
  case '-':
    return state * 2 - 2;
  default:
    return ERR;
  }
}

} // namespace morse