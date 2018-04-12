#include <algorithm>

#include <catch/catch.hpp>
#include <cppaikit/fsm/FSM.hpp>

namespace {

struct EventCounter {
  int timesEntered = 0;
  int timesExited = 0;
  int timesUpdated = 0;
  int accumulatedUpdates = 0;
};

class TestState : public aikit::fsm::State<> {
 public:
  explicit TestState(EventCounter* counter = nullptr) : mCounter(counter) {}

  void onEnter() override { if (mCounter) ++mCounter->timesEntered; }

  void onExit() override { if (mCounter) ++mCounter->timesExited; }

  void update(int updateData) override {
    if (mCounter) {
      ++mCounter->timesUpdated;
      mCounter->accumulatedUpdates += updateData;
    }
  }

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

    SECTION("adding state with existing id is ignored") {
      fsm.addState("state1", TestState());

      REQUIRE(fsm.size() == 3);
    }

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
    REQUIRE_FALSE(fsm.removeState("stateInvalid"));
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

    SECTION("when there is no previous state, call onExit() for the state being removed, current state is cleared") {
      REQUIRE_FALSE(fsm.hasPreviousState());
      REQUIRE(eventCounter.timesExited == 0);
      REQUIRE(eventCounter.timesEntered == 0);

      REQUIRE(fsm.removeState("state1"));
      REQUIRE(fsm.size() == 3);
      REQUIRE_FALSE(fsm.hasCurrentState());
      REQUIRE_FALSE(fsm.hasPreviousState());
      REQUIRE(eventCounter.timesExited == 1);
      REQUIRE(eventCounter.timesEntered == 0);
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

TEST_CASE("FSM can transition between states", "[state_machine], [fsm]") {
  aikit::fsm::FSM<> fsm;

  EventCounter eventCounterS1;
  EventCounter eventCounterS2;
  EventCounter eventCounterS3;

  fsm.addState("state1", TestState(&eventCounterS1));
  fsm.addState("state2", TestState(&eventCounterS2));
  fsm.addState("state3", TestState(&eventCounterS3));
  REQUIRE_FALSE(fsm.hasPreviousState());
  REQUIRE_FALSE(fsm.hasCurrentState());

  SECTION("a transition change state tracking") {
    REQUIRE(fsm.transitionTo("state1"));

    REQUIRE(fsm.hasCurrentState());
    REQUIRE(*fsm.currentStateId() == "state1");
    REQUIRE(eventCounterS1.timesExited == 0);
    REQUIRE(eventCounterS1.timesEntered == 1);
    REQUIRE(eventCounterS2.timesExited == 0);
    REQUIRE(eventCounterS2.timesEntered == 0);
    REQUIRE(eventCounterS3.timesExited == 0);
    REQUIRE(eventCounterS3.timesEntered == 0);

    SECTION("previous state is changed by successive transitions") {
      REQUIRE_FALSE(fsm.hasPreviousState());
      REQUIRE(fsm.hasCurrentState());

      REQUIRE(fsm.transitionTo("state2"));

      REQUIRE(fsm.hasPreviousState());
      REQUIRE(*fsm.previousStateId() == "state1");
      REQUIRE(*fsm.currentStateId() == "state2");

      REQUIRE(eventCounterS1.timesExited == 1);
      REQUIRE(eventCounterS1.timesEntered == 1);
      REQUIRE(eventCounterS2.timesExited == 0);
      REQUIRE(eventCounterS2.timesEntered == 1);
      REQUIRE(eventCounterS3.timesExited == 0);
      REQUIRE(eventCounterS3.timesEntered == 0);

      REQUIRE(fsm.transitionTo("state3"));

      REQUIRE(fsm.hasPreviousState());
      REQUIRE(*fsm.previousStateId() == "state2");
      REQUIRE(*fsm.currentStateId() == "state3");

      REQUIRE(eventCounterS1.timesExited == 1);
      REQUIRE(eventCounterS1.timesEntered == 1);
      REQUIRE(eventCounterS2.timesExited == 1);
      REQUIRE(eventCounterS2.timesEntered == 1);
      REQUIRE(eventCounterS3.timesExited == 0);
      REQUIRE(eventCounterS3.timesEntered == 1);
    }
  }

  SECTION("it is allowed to transition to current state") {
    REQUIRE(fsm.transitionTo("state1"));

    REQUIRE_FALSE(fsm.hasPreviousState());
    REQUIRE(fsm.hasCurrentState());
    REQUIRE(*fsm.currentStateId() == "state1");
    REQUIRE(eventCounterS1.timesExited == 0);
    REQUIRE(eventCounterS1.timesEntered == 1);

    REQUIRE(fsm.transitionTo("state1"));

    REQUIRE(fsm.hasPreviousState());
    REQUIRE(fsm.hasCurrentState());
    REQUIRE(*fsm.previousStateId() == "state1");
    REQUIRE(*fsm.currentStateId() == "state1");
    REQUIRE(eventCounterS1.timesExited == 1);
    REQUIRE(eventCounterS1.timesEntered == 2);
  }

  SECTION("transition to an invalid id is ignored") {
    REQUIRE_FALSE(fsm.transitionTo("stateInvalid"));

    REQUIRE_FALSE(fsm.hasPreviousState());
    REQUIRE_FALSE(fsm.hasCurrentState());
  }
}

TEST_CASE("FSM can transition to previous state") {
  aikit::fsm::FSM<> fsm;

  EventCounter eventCounterS1;
  EventCounter eventCounterS2;

  fsm.addState("state1", TestState(&eventCounterS1));
  fsm.addState("state2", TestState(&eventCounterS2));

  fsm.transitionTo("state1");

  REQUIRE_FALSE(fsm.hasPreviousState());
  REQUIRE(fsm.hasCurrentState());
  REQUIRE(*fsm.currentStateId() == "state1");
  REQUIRE(eventCounterS1.timesExited == 0);
  REQUIRE(eventCounterS1.timesEntered == 1);
  REQUIRE(eventCounterS2.timesExited == 0);
  REQUIRE(eventCounterS2.timesEntered == 0);

  SECTION("transition to previous state updates both current and previous state") {
    fsm.transitionTo("state2");

    REQUIRE(fsm.hasPreviousState());
    REQUIRE(fsm.hasCurrentState());
    REQUIRE(*fsm.currentStateId() == "state2");
    REQUIRE(*fsm.previousStateId() == "state1");
    REQUIRE(eventCounterS1.timesExited == 1);
    REQUIRE(eventCounterS1.timesEntered == 1);
    REQUIRE(eventCounterS2.timesExited == 0);
    REQUIRE(eventCounterS2.timesEntered == 1);

    REQUIRE(fsm.transitionToPreviousState());

    REQUIRE(fsm.hasPreviousState());
    REQUIRE(fsm.hasCurrentState());
    REQUIRE(*fsm.currentStateId() == "state1");
    REQUIRE(*fsm.previousStateId() == "state2");
    REQUIRE(eventCounterS1.timesExited == 1);
    REQUIRE(eventCounterS1.timesEntered == 2);
    REQUIRE(eventCounterS2.timesExited == 1);
    REQUIRE(eventCounterS2.timesEntered == 1);
  }

  SECTION("if there is no previous state, the call is ignored") {
    REQUIRE_FALSE(fsm.transitionToPreviousState());

    REQUIRE_FALSE(fsm.hasPreviousState());
    REQUIRE(fsm.hasCurrentState());
    REQUIRE(*fsm.currentStateId() == "state1");
    REQUIRE(eventCounterS1.timesExited == 0);
    REQUIRE(eventCounterS1.timesEntered == 1);
    REQUIRE(eventCounterS2.timesExited == 0);
    REQUIRE(eventCounterS2.timesEntered == 0);
  }
}

TEST_CASE("FSM can be updated", "[state_machine], [fsm]") {
  aikit::fsm::FSM<> fsm;

  EventCounter eventCounterS1;
  EventCounter eventCounterS2;

  fsm.addState("state1", TestState(&eventCounterS1));
  fsm.addState("state2", TestState(&eventCounterS2));

  REQUIRE_FALSE(fsm.hasPreviousState());
  REQUIRE_FALSE(fsm.hasCurrentState());

  SECTION("update call the update method of the current state") {
    fsm.transitionTo("state1");
    fsm.transitionTo("state2");

    REQUIRE(eventCounterS1.timesUpdated == 0);
    REQUIRE(eventCounterS2.timesUpdated == 0);
    REQUIRE(eventCounterS1.accumulatedUpdates == 0);
    REQUIRE(eventCounterS2.accumulatedUpdates == 0);

    fsm.update(2);

    REQUIRE(eventCounterS1.timesUpdated == 0);
    REQUIRE(eventCounterS2.timesUpdated == 1);
    REQUIRE(eventCounterS1.accumulatedUpdates == 0);
    REQUIRE(eventCounterS2.accumulatedUpdates == 2);

    fsm.update(2);
    fsm.update(2);
    REQUIRE(eventCounterS1.timesUpdated == 0);
    REQUIRE(eventCounterS2.timesUpdated == 3);
    REQUIRE(eventCounterS1.accumulatedUpdates == 0);
    REQUIRE(eventCounterS2.accumulatedUpdates == 6);
  }

  SECTION("call is ignored if FSM has no current state") {
    fsm.update(0);

    REQUIRE(eventCounterS1.timesUpdated == 0);
    REQUIRE(eventCounterS2.timesUpdated == 0);
    REQUIRE(eventCounterS1.accumulatedUpdates == 0);
    REQUIRE(eventCounterS2.accumulatedUpdates == 0);
  }
}

TEST_CASE("FSM can change initial state", "[state_machine], [fsm]") {
  aikit::fsm::FSM<> fsm;

  EventCounter eventCounter;

  fsm.addState("state1", TestState(&eventCounter));
  fsm.addState("state2", TestState(&eventCounter));

  REQUIRE(fsm.size() == 2);

  SECTION("initial state for FSM is undefined even after adding states") {
    REQUIRE_FALSE(fsm.hasCurrentState());
  }

  SECTION("initial state can be set") {
    fsm.setCurrentState("state1");

    REQUIRE(fsm.hasCurrentState());
    REQUIRE(*fsm.currentStateId() == "state1");

    SECTION("setting initial state does not call events for the state (no transition)") {
      REQUIRE(eventCounter.timesExited == 0);
      REQUIRE(eventCounter.timesEntered == 0);
    }
  }

  SECTION("setting initial state change to an invalid one is ignored") {
    REQUIRE_FALSE(fsm.setCurrentState("notValid"));
  }

  SECTION("setting initial state can change previous state") {
    fsm.setCurrentState("state1");

    REQUIRE_FALSE(fsm.hasPreviousState());
    REQUIRE(fsm.hasCurrentState());

    fsm.setCurrentState("state2");
    REQUIRE(fsm.hasPreviousState());
  }
}

TEST_CASE("FSM can list states and it's ids") {
  aikit::fsm::FSM<> fsm;

  fsm.addState("state1", TestState());
  fsm.addState("state2", TestState());
  fsm.addState("state3", TestState());

  SECTION("get number of states") {
    REQUIRE(fsm.size() == 3);
  }

  SECTION("list states") {
    const auto states = fsm.states();

    REQUIRE(states.size() == 3);
    // Test can be improved by testing the returned state objects
  }

  SECTION("list ids") {
    const auto stateIds = fsm.stateIds();

    const std::vector<std::string> rIdsVal{"state2", "state1", "state3"};
    const std::vector<const std::string*> rIds{&rIdsVal[0], &rIdsVal[1], &rIdsVal[2]};
    REQUIRE(std::is_permutation(rIds.begin(), rIds.end(), stateIds.begin(), stateIds.end(),
                                [](const auto& first, const auto& second) -> bool {
                                  return *second == *first;
                                }));
  }

  SECTION("check if a state exists") {
    REQUIRE(fsm.hasState("state1"));
    REQUIRE_FALSE(fsm.hasState("stateInvalid"));
  }

  SECTION("get state by id") {
    const auto stateById = fsm.getState("state1");

    REQUIRE(stateById != nullptr);

    SECTION("must return nullptr when state with id is not found") {
      const auto invalidStateById = fsm.getState("stateInvalid");

      REQUIRE(invalidStateById == nullptr);
    }
  }
}

}