#ifndef MORSE_WORLD_LINE_H_
#define MORSE_WORLD_LINE_H_

#include <cstdint>
#include <string>

#include "node.h"

namespace morse {

enum class LineState {
  IDLE,
  LOW,
  HIGH,
  BREAK,
};

class WorldLine : public Node {
private:
  uint64_t clock_ = 0;
  uint8_t prev_level_ = 0;

  LineState line_state_ = LineState::IDLE;

  double sum_dot_length_ = 0.0;
  uint32_t dot_count_ = 0;

  int16_t decoder_state_ = 0;
  std::string signals_ = {};
  std::string characters_ = {};

  double estimated_dot_length_ = 0.0;
  double confidence_score_ = 1.0;

public:
  WorldLine() {}
  WorldLine(const WorldLine &src);
  ~WorldLine() = default;

  bool Update(uint8_t level);

  void ChildRemoved();

  inline WorldLine *Next() { return reinterpret_cast<WorldLine *>(next_); }

  inline const std::string &GetSignals() const { return signals_; }
  inline const std::string &GetCharacters() const { return characters_; }
  inline double GetDotLength() const { return estimated_dot_length_; }
  inline double GetConfidence() const { return confidence_score_; }
  void NormalizeConfidence(double scale, bool do_square) {
    if (scale != 0.0) {
      confidence_score_ /= scale;
    }
    if (do_square) {
      confidence_score_ *= confidence_score_;
    }
  }

private:
  void Rise();
  void Drop();
  bool ExtendBreak();

  void AddDot();
  void AddDash();
  void AddBreak(bool update_dot_length);
  void AddSpace();
  void UpdateDotLength(uint32_t num_dots);

  static int16_t Decode(int16_t state, char signal);

  WorldLine *Fork(double weight = 0.5);
  void Terminate();
};

} // namespace morse

#endif // MORSE_WORLD_LINE_H_
