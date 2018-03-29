#include <cstdlib>
#include <iostream>

#include "cppaikit/fsm/FSM.hpp"

class MinimalState1 : public aikit::fsm::State<> {
 public:
  void onEnter() override { std::cout << "MinimalState1: onEnter" << std::endl; }
  void onExit() override { std::cout << "MinimalState1: onExit" << std::endl; }
  void update(int deltaTime) override { std::cout << "MinimalState1: " << deltaTime << std::endl; }
};

class MinimalState2 : public aikit::fsm::State<> {
 public:
  void onEnter() override { std::cout << "MinimalState2: onEnter" << std::endl; }
  void onExit() override { std::cout << "MinimalState2: onExit" << std::endl; }
  void update(int deltaTime) override { std::cout << "MinimalState2: " << deltaTime << std::endl; }
};

int main(int /*argc*/, const char* /*argv*/[]) {
  aikit::fsm::FSM fsm;

  MinimalState1 ms1t;
  fsm.addState("ms1", MinimalState1());
  fsm.addState("ms2", MinimalState2());
  fsm.addState("ms1-byReference", ms1t);

  fsm.setCurrentState("ms1");

  fsm.update(10);
  fsm.transitionTo("ms2");
  fsm.update(10);
  fsm.transitionTo("ms1-byReference");
  fsm.update(10);

  std::cout << "Current state: " << *fsm.currentStateId() << std::endl;

  return EXIT_SUCCESS;
}
