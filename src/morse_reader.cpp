#include "morse_reader.h"

#include <algorithm>
#include <curses.h>
#include <map>
#include <stdio.h>
#include <vector>

#include "node.h"
#include "world_line.h"

namespace morse {

MorseReader::MorseReader(Decoder *decoder)
    : decoder_{decoder}, clock_(0), state_(IDLE), last_interval_(0),
      estimated_dit_length_(0), dit_count_(0), sum_dit_length_(0) {
  observer_ = new Observer{};
  // attach the first world line
  observer_->Append(new WorldLine{});
}

MorseReader::~MorseReader() { delete decoder_; }

void MorseReader::Update(uint8_t level) {
  WorldLine *current = reinterpret_cast<WorldLine *>(observer_->next_);
  while (current != nullptr) {
    current->Update(level);
    current = current->Next();
  }
}

void MorseReader::Dump() {
  WorldLine *current = reinterpret_cast<WorldLine *>(observer_->next_);
  while (current != nullptr) {
    printf("%s: %f\n", current->GetCharacters().c_str(),
           current->GetConfidence());
    current = current->Next();
  }
}

void MorseReader::Dumpw(int width, int height, WINDOW *window) {
  WorldLine *current = reinterpret_cast<WorldLine *>(observer_->next_);
  if (current == nullptr) {
    wprintw(window, "Nothing to dump");
  }

  std::vector<WorldLine *> candidates;
  int irow = 0;
  double max_confidence = 0.0;
  while (current != nullptr) {
    max_confidence = std::max(max_confidence, current->GetConfidence());
    current = current->Next();
  }

  current = reinterpret_cast<WorldLine *>(observer_->next_);
  std::vector<WorldLine *> to_delete;
  while (current != nullptr) {
    if (max_confidence / current->GetConfidence() < 8.0) {
      candidates.push_back(current);
    } else {
      to_delete.push_back(current);
      current->Remove();
    }
    current = current->Next();
  }

  std::sort(candidates.begin(), candidates.end(), [](auto a, auto b) {
    return a->GetConfidence() > b->GetConfidence();
  });

  for (auto *world_line : to_delete) {
    delete world_line;
  }

  if (prev_dump_size_ > candidates.size()) {
    wclear(window);
  }
  prev_dump_size_ = candidates.size();

  bool do_square = num_scans_since_startup_++ > 10;

  for (auto *world_line : candidates) {
    double ratio = max_confidence / world_line->GetConfidence();
    mvwprintw(window, irow++, 0, "(%f) %f : %s", ratio,
              world_line->GetConfidence(), world_line->GetCharacters().c_str());
    mvwprintw(window, irow++, 4, "%s", world_line->GetSignals().c_str());
    world_line->NormalizeConfidence(max_confidence, do_square);
  }
}

} // namespace morse