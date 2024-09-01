#include "monitor.h"

#include <ncurses.h>

namespace morse {

Monitor::Monitor() {
  initscr();
  int height;
  int width;
  getmaxyx(stdscr, height, width);
  curs_set(0);
  dump_window_ = newwin(height - 19, width, 17, 0);
  height_ = height;
  width_ = width;
}

Monitor::~Monitor() {
  getmaxyx(stdscr, height_, width_);
  mvprintw(height_ - 1, 0, "press any key to exit");
  getch();
  endwin();
}

void Monitor::AddSignal(char signal) {
  mvprintw(irow_, icol_++, "%c", signal == '^' ? '*' : ' ');
  refresh();
  if (icol_ == width_ - 1) {
    ++irow_;
    icol_ = 1;
  }
}

void Monitor::Dump(MorseReader *reader) {
  // wclear(dump_window_);
  reader->Dumpw(0, 0, dump_window_);
  wrefresh(dump_window_);
}

} // namespace morse