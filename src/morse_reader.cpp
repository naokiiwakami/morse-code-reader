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

void MorseReader::Proceed() {
  ++clock_;
  if (estimated_dit_length_ == 0) {
    return;
  }
  if (state_ == LOW && clock_ > estimated_dit_length_ * 2.2) {
    decoder_->Break();
    state_ = BREAK;
  }
  if (state_ == BREAK && clock_ > estimated_dit_length_ * 5) {
    decoder_->Space();
    state_ = IDLE;
    clock_ = 0;
  }
}

void MorseReader::Rise() {
  if (state_ == LOW) {
    if (estimated_dit_length_ == 0) {
      int32_t this_interval = clock_;
      // if this is a break
      int32_t diff1 = last_interval_ - this_interval;
      int32_t diff2 = last_interval_ - this_interval * 3;
      if (diff1 * diff1 < diff2 * diff2) {
        decoder_->Dit();
        dit_count_ += 1;
      } else {
        decoder_->Dah();
        dit_count_ += 3;
      }
      sum_dit_length_ += last_interval_;
    }
    dit_count_ += 1;
    sum_dit_length_ += clock_;
    estimated_dit_length_ = (uint32_t)(sum_dit_length_ / dit_count_ + 0.5);
  }
  state_ = HIGH;
  clock_ = 0;
}

void MorseReader::Fall() {
  if (estimated_dit_length_ == 0) {
    last_interval_ = clock_;
  } else {
    // printf("%d", estimated_dit_length_);
    if (clock_ < estimated_dit_length_ * 1.5) {
      decoder_->Dit();
      dit_count_ += 1;
      sum_dit_length_ += clock_;
    } else {
      decoder_->Dah();
      dit_count_ += 3;
      sum_dit_length_ += clock_;
    }
    estimated_dit_length_ = (uint32_t)(sum_dit_length_ / dit_count_ + 0.5);
  }
  clock_ = 0;
  state_ = LOW;
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
  // std::map<double, std::vector<WorldLine*>> candidates;
  std::vector<WorldLine *> candidates;
  int irow = 0;
  while (current != nullptr) {
    candidates.push_back(current);
    current = current->Next();
  }
  std::sort(candidates.begin(), candidates.end(), [](auto a, auto b) {
    return a->GetConfidence() > b->GetConfidence();
  });

  if (prev_dump_size_ > candidates.size()) {
    wclear(window);
  }
  prev_dump_size_ = candidates.size();

  bool init = true;
  for (auto *world_line : candidates) {
    if (init) {
      attron(COLOR_PAIR(0));
    }
    mvwprintw(window, irow++, 0, "%f : %s", world_line->GetConfidence(),
              world_line->GetCharacters().c_str());
    mvwprintw(window, irow++, 4, "%s", world_line->GetSignals().c_str());
    if (init) {
      attroff(COLOR_PAIR(0));
    }
    init = false;
  }
}

} // namespace morse