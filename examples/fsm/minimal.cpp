#include <cstdlib>
#include <iostream>

#include "cppaikit/fsm/FSM.hpp"

// Define the state class (or classes) to be used on the FSM, they MUST inherit from fsm::State
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
  // Instance a FSM
  aikit::fsm::FSM<> fsm;

  // Add states to the FSM
  MinimalState1 ms1t;
  fsm.addState("ms1", MinimalState1()); // States can be added by move
  fsm.addState("ms2", MinimalState2());
  fsm.addState("ms1-byReference", ms1t); // States can also be added by copy

  // Set the initial state of the FSM
  fsm.setCurrentState("ms1");

  // A call to update() on FSM will forward the arguments to update() of the current state
  fsm.update(10); // In this case update() will be called for the "ms1" state

  // Make transition to another state
  fsm.transitionTo("ms2");
  fsm.update(10); // Now update() will be called for state "ms2"

  fsm.transitionTo("ms1-byReference");
  fsm.update(10);

  std::cout << "Current state: " << *fsm.currentStateId() << std::endl;
  // Will output "Current state: ms1-byReference"

  return EXIT_SUCCESS;
}
