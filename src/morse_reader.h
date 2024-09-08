#ifndef MORSE_READER_H_
#define MORSE_READER_H_

#include <ncurses.h>

#include <cstdint>
#include <vector>

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
  uint32_t clock_;
  ReaderState state_;
  int32_t last_interval_;
  uint32_t estimated_dit_length_;
  uint32_t dit_count_;
  double sum_dit_length_;

  Observer *observer_;

  size_t num_scans_since_startup_ = 0;

  size_t prev_dump_size_ = 0;

public:
  MorseReader();
  virtual ~MorseReader();

  bool Update(uint8_t level);

  double GetEstimatedDotLength();

  void Dump();
  void Dumpw(int width, int height, WINDOW *window);
};

} // namespace morse

#endif // MORSE_READER_H_