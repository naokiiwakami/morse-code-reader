#include "morse_reader.h"

MorseReader::MorseReader(MorseDecoder *decoder)
    : decoder_{decoder}, clock_(0), state_(IDLE), estimated_dit_length_(7),
      dit_count_(0), sum_dit_length_(0) {}

MorseReader::~MorseReader() { delete decoder_; }

void MorseReader::Proceed() {
  ++clock_;
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
    dit_count_ += 1;
    sum_dit_length_ += clock_;
    estimated_dit_length_ = (uint32_t)(sum_dit_length_ / dit_count_ + 0.5);
  }
  state_ = HIGH;
  clock_ = 0;
}

void MorseReader::Fall() {
  if (clock_ < estimated_dit_length_ * 1.5) {
    decoder_->Dit();
    dit_count_ += 1;
    sum_dit_length_ += clock_;
  } else {
    decoder_->Dah();
    dit_count_ += 3;
    sum_dit_length_ += clock_;
  }
  state_ = LOW;
  clock_ = 0;
  estimated_dit_length_ = (uint32_t)(sum_dit_length_ / dit_count_ + 0.5);
}
