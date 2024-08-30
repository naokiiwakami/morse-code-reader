#ifndef EVENT_LISTENER_H_
#define EVENT_LISTENER_H_

#include <stdio.h>

#include <string>

namespace morse {

enum class EventType {
  SIGNAL,
  OUT,
};

class EventListener {
public:
  virtual ~EventListener() = default;
  virtual void OnEvent(EventType event_type, const std::string &event) = 0;
};

class DefaultEventListener : public EventListener {
public:
  void OnEvent(EventType event_type, const std::string &event) {
    if (event_type == EventType::OUT) {
      printf(" %s  ", event.c_str());
    } else {
      printf("%s", event.c_str());
    }
    fflush(stdout);
  }
};

} // namespace morse

#endif // EVENT_LISTENER_H_
