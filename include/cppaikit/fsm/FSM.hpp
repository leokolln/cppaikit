#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "State.hpp"

namespace aikit::fsm {

/**
 * Implementation for a Finite State Machine.
 * @tparam TId Type for the id of a state. Defaults to std::string.
 * @tparam TState Base type for the states managed by the machine. Defaults to fsm::State<int>.
 * @sa fsm::State
 */
template<typename TId = std::string, typename TState = State<int>>
class FSM {
 public:
  typedef TId Id_type;
  typedef typename TState::UpdateData_type UpdateData_type;

  /**
   * Adds a new state to the FSM.
   * The machine will keep a copy of the state that will be destroyed only when it is removed with
   * removeState() or the machine is destroyed.
   * @param id Identification of the state being added, this is used to reference the state in all other methods.
   * @param state The state being added. It must inherit from the class fsm::State.
   * @note If any state with equivalent \a id already exists, does nothing.
   */
  template<typename TNewState>
  void addState(TId id, TNewState&& state) {
    using TNewStateNoRef = std::remove_reference_t<TNewState>;
    static_assert(std::is_base_of_v<TState, TNewStateNoRef>,
                  "addState() is only callable with state that is derived from TState");

    mStates.try_emplace(std::move(id), std::make_unique<TNewStateNoRef>(std::forward<TNewState>(state)));
  }

  /**
   * Remove a state from the FSM.
   * Removes and destroys the state associated with \a id.
   * @param id Identification of the state being removed.
   * @return Returns true only when a state with associated \a id is found and consequentially removed.
   * @note If \a id refers to FSM current state and there is no previous state, fsm::State::onExit() will be
   * called before removing it.
   * @note If \a id refers to FSM current state, will transition to previous (if any) before removing it.
   * @note After a transition to previous because \a id refers to current, the previous will be equal to
   * current (previous state will not be cleared or changed).
   * @attention If \a id refers to FSM previous state, will clear it before removing and no transition will happen.
   * @attention If \a id refers to a state that is both current and previous, fsm::State::onExit() will be
   * called for the state before removing it. Both current and previous will be cleared.
   */
  bool removeState(const TId& id) {
    // TODO Reimplement, find first then determine actions if it is current or previous or both
    if (hasCurrentState() && (*currentStateId() == id)) {
      if (hasPreviousState()) {
        if (mCurrentState.id == mPreviousState.id) { // No need to compare values again, just check if point to same
          mCurrentState.state->onExit();
          mCurrentState.clear();
          mPreviousState.clear();
        } else {
          transitionToPreviousState();
          mPreviousState = mCurrentState;
        }
      } else {
        mCurrentState.state->onExit();
        mCurrentState.clear();
      }
    } else if (hasPreviousState() && (*previousStateId() == id)) {
      mPreviousState.clear();
    }

    return mStates.erase(id) > 0;
  }

  /**
   * Transition to a state.
   * fsm::State::onExit() will be called for the current state (if any), state with the associated \a id will be set
   * as current and fsm::State::onEnter() will be called on it.
   * Previous state will be set with the state that was current.
   * @param id Identification of the state that will be transitioned to.
   * @return True if \a id was found on the FSM.
   * @note If \a id was not found, the call is ignored.
   * @note The order of operations during transition is: Call fsm::State::onExit() for current
   * state (if any), set previous state with the current state (if any), set current state with the \a id
   * state, call fsm::State::onEnter() for the new current state.
   * @sa fsm::State::onExit()
   * @sa fsm::State::onEnter()
   */
  bool transitionTo(const TId& id) {
    auto found = mStates.find(id);
    const bool foundStateId = (found != mStates.end());

    if (foundStateId) {
      if (hasCurrentState()) {
        mCurrentState.state->onExit();
        mPreviousState = mCurrentState;
      }

      mCurrentState = {&found->first, found->second.get()};
      mCurrentState.state->onEnter();
    }

    return foundStateId;
  }

  /**
   * Transition to FSM previous state.
   * @return True if there was a previous state to transition to.
   * @note If there is no previous state, the call is ignored.
   * @sa transitionTo()
   */
  bool transitionToPreviousState() {
    const bool hasPrevious = hasPreviousState();
    if (hasPrevious) {
      transitionTo(*mPreviousState.id);
    }

    return hasPrevious;
  }

  /**
   * Update FSM and it's current state.
   * @param updateData The data that will be passed during the call fsm::State::update() on the current state.
   * @note If there is no current state, the call is ignored.
   * @sa fsm::State::update()
   */
  void update(UpdateData_type updateData) {
    if (hasCurrentState()) {
      mCurrentState.state->update(updateData);
    }
  }

  /**
   * Set the current state of the FSM.
   * This method should be preferably used to define the initial state of the machine or in any other circumstance
   * where calls to fsm::State::onExit() and fsm::State::onEnter() are undesirable.
   * @param id Identification of the state that will be set as current.
   * @return True if \a id was found on the FSM.
   * @note If \a id was not found, the call is ignored.
   * @note This method can be called even if no previous state was set as current.
   * @note Previous state will be set with the state currently set (if any).
   * @attention fsm::State::onExit() will not be called for the current state (if any).
   * @attention fsm::State::onEnter() will not be called for the state being set as current.
   */
  bool setCurrentState(const TId& id) {
    auto found = mStates.find(id);
    const bool foundStateId = (found != mStates.end());

    if (foundStateId) {
      if (mCurrentState.isSet()) {
        mPreviousState = mCurrentState;
      }

      mCurrentState = {&found->first, found->second.get()};
    }

    return foundStateId;
  }

  /**
   * Check if the FSM has a current state set.
   * @return True if there is a current state.
   */
  bool hasCurrentState() const {
    return mCurrentState.isSet();
  }

  /**
   * The identification of the current state of the FSM.
   * @return Id of the current state.
   * @attention The FSM must have a current state that was previously set by either transitionTo() or setCurrentState().
   */
  const TId* currentStateId() const {
    return mCurrentState.id;
  }

  /**
   * The current state of the FSM.
   * @return Current state of the FSM, can be nullptr if no state is set.
   */
  const TState* currentState() const {
    return mCurrentState.state;
  }

  /**
   * The current state of the FSM.
   * @return Current state of the FSM, can be nullptr if no state is set.
   */
  TState* currentState() {
    return mCurrentState.state;
  }

  /**
   * Check if the FSM has a state previously set.
   * @return True if there is a previous state.
   */
  bool hasPreviousState() const {
    return mPreviousState.isSet();
  }

  /**
   * The identification of the previous state of the FSM.
   * @return Current state of the FSM, can be nullptr if no state is set.
   */
  const TId* previousStateId() const {
    return mPreviousState.id;
  }

  /**
   * The previous state of the FSM.
   * @return Current state of the FSM, can be nullptr if no state is set.
   */
  const TState* previousState() const {
    return mPreviousState.state;
  }

  /**
   * The previous state of the FSM.
   * @return Current state of the FSM, can be nullptr if no state is set.
   */
  TState* previousState() {
    return mPreviousState.state;
  }

  /**
   * Lists the ids of all states in the FSM.
   * @return List of states ids in the FSM.
   */
  std::vector<TId*> statesIds() const {
    std::vector<TId*> ids;
    ids.reserve(mStates.size());

    for (const auto& stateMap : mStates) {
      ids.emplace_back(&stateMap.first);
    }

    return ids;
  }

  /**
   * Lists all the states in the FSM.
   * @return List of states in the FSM.
   */
  std::vector<TState*> states() const {
    std::vector<TState*> retStates;
    retStates.reserve(mStates.size());

    for (const auto& stateMap : mStates) {
      retStates.emplace_back(stateMap->second.get());
    }

    return retStates;
  }

  /**
   * Checks if the FSM has a state with a given id.
   * @param id The identification of a state.
   * @return True if the FSM has a state with the given id.
   */
  bool hasState(const TId& id) const {
    return mStates.count(id) > 0;
  }

  /**
   * Get the state with the associated id.
   * @param id The identification of a state.
   * @return A state with the given id.
   * @warning Will return nullptr if there is no state with the id.
   */
  const TState* getState(const TId& id) const {
    const auto found = mStates.find(id);
    if (found != mStates.end()) {
      return found->second.get();
    } else {
      return nullptr;
    }
  }

  /**
   * Number of states in the FSM.
   * @return The number of states in the FSM.
   */
  std::size_t size() const {
    return mStates.size();
  }

 private:
  struct StateRef {
    const TId* id = nullptr; ///< The id of the FSM's current state.
    TState* state = nullptr; ///< The FSM's current state.

    bool isSet() const { return state != nullptr; }

    void clear() {
      id = nullptr;
      state = nullptr;
    }
  };

  std::map<TId, std::unique_ptr<TState>> mStates; ///< Mapping of states and associated ids.
  StateRef mPreviousState{};
  StateRef mCurrentState{};
};

}
