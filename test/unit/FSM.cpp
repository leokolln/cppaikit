#include <catch/catch.hpp>
#include <cppaikit/fsm/FSM.hpp>

namespace {

struct EventCounter {
  int timesEntered = 0;
  int timesExited = 0;
  int timesUpdates = 0;
};

class TestState : public aikit::fsm::State<> {
 public:
  explicit TestState(EventCounter* counter = nullptr) : mCounter(counter) {}

  void onEnter() override { if (mCounter) ++mCounter->timesEntered; }

  void onExit() override { if (mCounter) ++mCounter->timesExited; }

  void update(int updateData) override { if (mCounter) ++mCounter->timesUpdates; }

  EventCounter* mCounter;
};

TEST_CASE("FSM can have states added and removed", "[state_machine], [fsm]") {
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

TEST_CASE("Removing states from FSM can change state tracking", "[state_machine], [fsm]") {
  aikit::fsm::FSM fsm;

  EventCounter eventCounter;

  fsm.addState("state1", TestState(&eventCounter));
  fsm.addState("state2", TestState(&eventCounter));
  fsm.addState("state3", TestState(&eventCounter));
  fsm.addState("state4", TestState(&eventCounter));

  REQUIRE(fsm.size() == 4);

  SECTION("removing state not found on FSM is ignored") {
    REQUIRE_FALSE(fsm.removeState("stateNotAdded"));
    REQUIRE(fsm.size() == 4);
  }

  SECTION("removing existing state is confirmed by return") {
    REQUIRE(fsm.removeState("state1"));
    REQUIRE(fsm.size() == 3);
  }

  SECTION("it is possible to remove state that is set as current") {
    fsm.setCurrentState("state1");
    REQUIRE(*fsm.currentStateId() == "state1");

    SECTION("previous state is considered during removal, if it is set") {
      SECTION("when current and previous state are the same, only onExit() is called and both states cleared") {
        fsm.transitionTo("state1");
        REQUIRE(fsm.hasPreviousState());
        REQUIRE(fsm.currentState() == fsm.previousState());
        REQUIRE(eventCounter.timesExited == 1);
        REQUIRE(eventCounter.timesEntered == 1);

        REQUIRE(fsm.removeState("state1"));
        REQUIRE(fsm.size() == 3);
        REQUIRE_FALSE(fsm.hasCurrentState());
        REQUIRE_FALSE(fsm.hasPreviousState());
        REQUIRE(eventCounter.timesExited == 2);
        REQUIRE(eventCounter.timesEntered == 1);
      }

      SECTION("when current and previous are different, transition to previous and set both to same state") {
        fsm.transitionTo("state2");
        REQUIRE(fsm.hasPreviousState());
        REQUIRE(*fsm.previousStateId() == "state1");
        REQUIRE(fsm.currentState() != fsm.previousState());
        REQUIRE(eventCounter.timesExited == 1);
        REQUIRE(eventCounter.timesEntered == 1);

        REQUIRE(fsm.removeState("state2"));
        REQUIRE(fsm.size() == 3);
        REQUIRE(fsm.hasCurrentState());
        REQUIRE(fsm.hasPreviousState());
        REQUIRE(fsm.currentState() == fsm.previousState());
        REQUIRE(*fsm.currentStateId() == "state1");
        REQUIRE(eventCounter.timesExited == 2);
        REQUIRE(eventCounter.timesEntered == 2);
      }
    }

    SECTION("when there is no previous state set, call onExit() for it before removal") {

    }
  }

  SECTION("it is possible to remove state that is set as previous") {
    fsm.setCurrentState("state1");
    REQUIRE(*fsm.currentStateId() == "state1");

    SECTION("if previous state is different from current, no transition happens and previous is cleared") {
      fsm.transitionTo("state2");
      REQUIRE(fsm.hasPreviousState());
      REQUIRE(*fsm.previousStateId() == "state1");
      REQUIRE(fsm.currentState() != fsm.previousState());
      REQUIRE(eventCounter.timesExited == 1);
      REQUIRE(eventCounter.timesEntered == 1);

      REQUIRE(fsm.removeState("state1"));
      REQUIRE(fsm.size() == 3);
      REQUIRE(fsm.hasCurrentState());
      REQUIRE_FALSE(fsm.hasPreviousState());
      REQUIRE(*fsm.currentStateId() == "state2");
      REQUIRE(eventCounter.timesExited == 1);
      REQUIRE(eventCounter.timesEntered == 1);
    }
  }
}

TEST_CASE("FSM is created with no initial state set", "[state_machine], [fsm]") {
}

TEST_CASE("FSM can transition between states", "[state_machine], [fsm]") {

}

}