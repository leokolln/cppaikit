#pragma once

namespace aikit::fsm {

template<typename TUpdateData = int>
class State {
 public:
  typedef TUpdateData UpdateData_type;

  virtual ~State() = default;

  virtual void onEnter() {};
  virtual void onExit() {};
  virtual void update(TUpdateData updateData) = 0;
};

}
