#include <catch/catch.hpp>
#include <cppaikit/fsm/FSM.hpp>

class TestState : public aikit::fsm::State<> {
 public:
  void onEnter() override { ++timesEntered; }

  void onExit() override { ++timesExited; }

  void update(int updateData) override { ++timesUpdates; }

  int timesEntered = 0;
  int timesExited = 0;
  int timesUpdates = 0;
};

TEST_CASE("FSM can have states added and removed", "[state_machine],[fsm]") {
  aikit::fsm::FSM fsm;

  REQUIRE(fsm.size() == 0);

  SECTION("states can be added by copy") {
    TestState stateToCopy;
    fsm.addState("copiedState", stateToCopy);

    REQUIRE(fsm.hasState("copiedState"));
  }

  SECTION("states can be added by move") {
    fsm.addState("movedState", TestState());

    REQUIRE(fsm.hasState("movedState"));
  }

  SECTION("adding states increase the number of states on the FSM") {
    fsm.addState("state1", TestState());
    fsm.addState("state2", TestState());
    fsm.addState("state3", TestState());

    REQUIRE(fsm.size() == 3);
    REQUIRE(fsm.hasState("state1"));
    REQUIRE(fsm.hasState("state2"));
    REQUIRE(fsm.hasState("state3"));

    SECTION("removing states decrease the number of states on the FSM") {
      fsm.removeState("state1");

      REQUIRE(fsm.size() == 2);
      REQUIRE_FALSE(fsm.hasState("state1"));
      REQUIRE(fsm.hasState("state2"));
      REQUIRE(fsm.hasState("state3"));
    }
  }
}