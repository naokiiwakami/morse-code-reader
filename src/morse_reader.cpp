#include <stdio.h>

#include "morse_reader.h"

namespace morse {

MorseReader::MorseReader(MorseDecoder *decoder)
    : decoder_{decoder}, clock_(0), state_(IDLE), last_interval_(0),
      estimated_dit_length_(0), dit_count_(0), sum_dit_length_(0) {}

MorseReader::~MorseReader() { delete decoder_; }

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

} // namespace morse