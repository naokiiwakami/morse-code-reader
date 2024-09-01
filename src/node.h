#ifndef MORSE_NODE_H_
#define MORSE_NODE_H_

namespace morse {

class Node {
public:
  virtual ~Node() = default;

  Node *prev_ = nullptr;
  Node *next_ = nullptr;

  void Append(Node *new_node) {
    new_node->next_ = next_;
    if (next_ != nullptr) {
      next_->prev_ = new_node;
    }
    new_node->prev_ = this;
    next_ = new_node;
  }

  void Insert(Node *new_node) {
    new_node->prev_ = prev_;
    if (prev_ != nullptr) {
      prev_->next_ = new_node;
    }
    new_node->next_ = this;
    prev_ = new_node;
  }

  virtual void ChildRemoved() = 0;
};

} // namespace morse

#endif // MORSE_NODE_H_