#ifndef MORSE_MONITOR_H_
#define MORSE_MONITOR_H_

#include <ncurses.h>

#include "morse_reader.h"

namespace morse {

class Monitor {
private:
  int irow_ = 0;
  int icol_ = 1;
  int width_;
  int height_;
  WINDOW *dump_window_;

public:
  Monitor();
  ~Monitor();

  void AddSignal(char signal);
  void Dump(MorseReader *reader);
};

} // namespace morse

#endif // MORSE_MONITOR_H_
