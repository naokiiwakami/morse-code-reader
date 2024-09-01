#ifndef MORSE_READER_H_
#define MORSE_READER_H_

#include <ncurses.h>

#include <cstdint>
#include <vector>

#include "decoder.h"
#include "node.h"

namespace morse {

enum ReaderState {
  IDLE,
  HIGH,
  LOW,
  BREAK,
};

class Observer : public Node {
public:
  ~Observer() = default;

  void ChildRemoved() {
    // TODO(Naoki): Collect available results
  }
};

class MorseReader {
private:
  Decoder *decoder_;
  uint32_t clock_;
  ReaderState state_;
  int32_t last_interval_;
  uint32_t estimated_dit_length_;
  uint32_t dit_count_;
  float sum_dit_length_;

  Observer *observer_;

  size_t prev_dump_size_ = 0;

public:
  MorseReader(Decoder *decoder);
  virtual ~MorseReader();

  void Update(uint8_t level);

  void Proceed();
  void Rise();
  void Fall();

  void Dump();
  void Dumpw(int width, int height, WINDOW *window);
};

} // namespace morse

#endif // MORSE_READER_H_