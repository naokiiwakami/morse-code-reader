#include "world_line.h"

namespace morse {

WorldLine::WorldLine(const WorldLine &src)
    : clock_(src.clock_), prev_level_(src.prev_level_),
      line_state_(src.line_state_), sum_dot_length_(src.sum_dot_length_),
      dot_count_(src.dot_count_),
      decoder_state_(src.decoder_state_), signals_{src.signals_},
      characters_{src.characters_},
      estimated_dot_length_(src.estimated_dot_length_),
      confidence_score_(src.confidence_score_) {}

bool WorldLine::Update(uint8_t level) {
  ++clock_;
  auto prev_level = prev_level_;
  prev_level_ = level;
  bool changed = false;
  if (level > 0 && prev_level == 0) {
    Rise();
    changed = true;
  } else if (level == 0) {
    if (prev_level > 0) {
      Drop();
      changed = true;
    } else {
      changed = ExtendBreak();
    }
  }
  return changed;
}

void WorldLine::Rise() {
  auto prev_line_state = line_state_;
  line_state_ = LineState::HIGH;
  if (prev_line_state == LineState::LOW) {
    if (clock_ < estimated_dot_length_ * 0.3) {
      Terminate();
      return;
    }
    if (clock_ > estimated_dot_length_ * 1.5) {
      if (clock_ < estimated_dot_length_ * 2.5) {
        double probability = (((double)clock_) / estimated_dot_length_) / 3.0;
        // double probability = 0.5;
        auto fork = Fork(probability);
        fork->AddBreak(true);
      } else {
        AddBreak(true);
        if (clock_ > estimated_dot_length_ * 5) {
          AddSpace();
        }
        return;
      }
    }
    sum_dot_length_ += clock_;
    dot_count_ += 1;
    estimated_dot_length_ = sum_dot_length_ / dot_count_;
  }
  if (prev_line_state == LineState::BREAK &&
      clock_ > estimated_dot_length_ * 10) {
    confidence_score_ *= 0.5;
  }
  clock_ = 0;
}

void WorldLine::Drop() {
  line_state_ = LineState::LOW;
  if (dot_count_ == 0) {
    Fork()->AddDash();
    AddDot();
  } else {
    if (clock_ < estimated_dot_length_ * 0.3) {
      Terminate();
      return;
    } else if (clock_ < estimated_dot_length_ * 2.3) {
      if (clock_ > estimated_dot_length_ * 1.5) {
        double probability = (((double)clock_) / estimated_dot_length_) / 3.0;
        Fork(probability)->AddDash();
      }
      AddDot();
    } else if (clock_ > estimated_dot_length_ * 7) {
      Terminate();
      return;
    } else {
      AddDash();
    }
  }
  clock_ = 0;
}

bool WorldLine::ExtendBreak() {
  if (dot_count_ == 0) {
    return false;
  }
  bool changed = false;
  if (line_state_ == LineState::LOW && clock_ > estimated_dot_length_ * 5) {
    AddBreak(false);
    // AddSpace();
    line_state_ = LineState::BREAK;
    changed = true;
  }
  if (line_state_ == LineState::BREAK && clock_ > estimated_dot_length_ * 7) {
    AddSpace();
    line_state_ = LineState::IDLE;
    changed = true;
  }
  return changed;
}

void WorldLine::AddDot() {
  signals_.push_back('.');
  UpdateDotLength(1);
  if ((decoder_state_ = Decode(decoder_state_, '.')) == 0) {
    Terminate();
  }
}

void WorldLine::AddDash() {
  signals_.push_back('-');
  UpdateDotLength(3);
  if ((decoder_state_ = Decode(decoder_state_, '-')) == 0) {
    Terminate();
  }
}

void WorldLine::AddBreak(bool update_dot_length) {
  signals_.push_back(' ');
  if (update_dot_length) {
    UpdateDotLength(3);
  }
  if ((decoder_state_ = Decode(decoder_state_, ' ')) == 0) {
    Terminate();
  }
  characters_.push_back(static_cast<char>(decoder_state_));
  decoder_state_ = 0;
}

void WorldLine::AddSpace() {
  signals_.push_back(' ');
  characters_.push_back(' ');
}

void WorldLine::UpdateDotLength(uint32_t num_dots) {
  sum_dot_length_ += clock_;
  dot_count_ += num_dots;
  estimated_dot_length_ = sum_dot_length_ / dot_count_;
  clock_ = 0;
}

WorldLine *WorldLine::Fork(double weight) {
  auto *clone = new WorldLine(*this);
  Insert(clone);
  clone->confidence_score_ *= weight;
  confidence_score_ *= (1 - weight);
  // TODO(Naoki): notify
  return clone;
}

void WorldLine::Terminate() {
  // just put the confidence down to the floor so that the reader
  // will kill this world line at the end of the cycle.
  confidence_score_ = 0.0;
  prev_->ChildRemoved();
}

void WorldLine::ChildRemoved() {
  if (next_ == nullptr) {
    prev_->ChildRemoved();
  }
}

int16_t WorldLine::Decode(int16_t state, char signal) {
  static const unsigned char table[] = {
      0x03, 0x97, 0xd3, 0xa7, 0x87, 0xbb, 0xb7, 0xcf, 0xd7, 0xcb, 0xdf, 0x93,
      0xaf, 0x9f, 0xbf, 0xa3, 0xda, 0x98, 0x03, 0xb0, 0x01, 0xc0, 0xaa, 0x8b,
      0xe1, 0x8c, 0xe5, 0xeb, 0xc4, 0x01, 0x03, 0x54, 0x50, 0x00, 0x4c, 0x00,
      0x00, 0x01, 0x48, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x45, 0x5a,
      0x74, 0x3c, 0x00, 0x00, 0x00, 0x22, 0x00, 0x5c, 0x02, 0x00, 0x00, 0x61,
      0x00, 0x64, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00,
      0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x68,
  };
  static const int16_t kTableSize =
      static_cast<int16_t>((sizeof(table) / sizeof(*table)));
  if (state > 0 || -state >= kTableSize) {
    return 0;
  }
  int16_t value = table[-state];
  switch (signal) {
  case '\0':
  case ' ':
    return value >= 0x4 ? (value >> 2) + ' ' : 0;
  case '.':
    return value & 0x1 ? state * 2 - 1 : 0;
  case '-':
    return value & 0x2 ? state * 2 - 2 : 0;
  default:
    return 0;
  }
}

} // namespace morse