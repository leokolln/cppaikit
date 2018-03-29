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
 * @tparam TUpdateData Type for the data being passed on update() and fsm::State::update(). Defaults to int.
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
   * @param state The state being added. It must inherit from the class fsm::State with the
   * template parameter TUpdateData equal to the FSM.
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
   * @note If \a id refers to FSM current state, will transition to previous (if any) before removing it.
   * @note After a transition to previous because \a id refers to current, the previous will be equal to
   * current (previous state will not be cleared or changed).
   * @attention If \a id refers to FSM previous state, will clear it before removing and no transition will happen.
   * @attention If \a id refers to a state that is both current and previous, fsm::State::onExit() will be
   * called for the state before removing it. Both current and previous will be cleared.
   */
  bool removeState(const TId& id) {
    // TODO Test complete behavior of removeState
    if (mCurrentState.isSet() && (currentStateId() == id)) {
      if (mPreviousState.isSet()) {
        if (mCurrentState.id == mPreviousState.id) { // No need to compare values again, just check if point to same
          mCurrentState.state->onExit();
          mCurrentState.clear();
          mPreviousState.clear();
        } else {
          transitionToPreviousState();
          mPreviousState = mCurrentState;
        }
      } else {
        mCurrentState.clear();
      }
    } else if (mPreviousState.isSet() && (previousStateId() == id)) {
      mPreviousState.clear();
    }

    return mStates.erase(id) > 0;
  }

  /**
   * Transition to a state.
   * fsm::State::onExit() will be called for the current state (if any), state with the associated \a id will be set
   * as current and fsm::State::onEnter() will be called on it.
   * @param id Identification of the state that will be transitioned to.
   * @note This method can be called even if no previous state was set as current.
   * @attention It is required that a state with the associated \a id is found on the FSM.
   * @sa fsm::State::onExit()
   * @sa fsm::State::onEnter()
   */
  void transitionTo(const TId& id) {
    auto found = mStates.find(id);
    assert(found != mStates.end()
               && "No state with informed id was found on the FSM. Check if it was added or if id is incorrect");

    if (mCurrentState.isSet()) {
      mCurrentState.state->onExit();
      mPreviousState = mCurrentState;
    }

    mCurrentState = {&(found->first), found->second.get()};
    mCurrentState.state->onEnter();
  }

  /**
   * Transition to FSM previous state.
   * @attention The FSM must have a previous state set by consecutive calls to transitionTo() or setCurrentState().
   * @sa transitionTo()
   */
  void transitionToPreviousState() {
    assert(mPreviousState.isSet()
               && "No previous state. Check if transitionTo() or setCurrentState() were called more than once");
    transitionTo(mPreviousState.id);
  }

  /**
   * Update FSM and it's current state.
   * @param updateData The data that will be passed during the call fsm::State::update() on the current state.
   * @attention The FSM must have a current state that was previously set by either transitionTo() or setCurrentState().
   * @sa fsm::State::update()
   */
  void update(UpdateData_type updateData) {
    assert(mCurrentState.isSet()
               && "The FSM has no current state. Check if transitionTo() or setCurrentState() were called");
    mCurrentState.state->update(updateData);
  }

  /**
   * Set the current state of the FSM.
   * This method should be preferably used to define the initial state of the machine or in any other circumstance
   * where calls to fsm::State::onExit() and fsm::State::onEnter() are undesirable.
   * @param id Identification of the state that will be set as current.
   * @note This method can be called even if no previous state was set as current.
   * @note Previous state will be set with the state currently set (if any).
   * @attention fsm::State::onExit() will not be called for the current state (if any).
   * @attention fsm::State::onEnter() will not be called for the state being set as current.
   * @attention It is required that a state with the associated \a id is found on the FSM.
   */
  void setCurrentState(const TId& id) {
    auto found = mStates.find(id);
    assert(found != mStates.end()
               && "No state with informed id was found on the FSM. Check if it was added or if id is incorrect");

    if (mCurrentState.isSet()) {
      mPreviousState = mCurrentState;
    }

    mCurrentState = {&(found->first), found->second.get()};
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
  const TId& currentStateId() const {
    assert(mCurrentState.isSet()
               && "The FSM has no current state. Check if transitionTo() or setCurrentState() were called");
    return *mCurrentState.id;
  }

  /**
   * The current state of the FSM.
   * @return Current state of the FSM.
   * @attention The FSM must have a current state that was previously set by either transitionTo() or setCurrentState().
   */
  const TState& currentState() const {
    assert(mCurrentState.isSet()
               && "The FSM has no current state. Check if transitionTo() or setCurrentState() were called");
    return *mCurrentState.state;
  }

  /**
   * The current state of the FSM.
   * @return Current state of the FSM.
   * @attention The FSM must have a current state that was previously set by either transitionTo() or setCurrentState().
   */
  TState& currentState() {
    assert(mCurrentState.isSet()
               && "The FSM has no current state. Check if transitionTo() or setCurrentState() were called");
    return *mCurrentState.state;
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
   * @return Id of the previous state.
   * @attention The FSM must have a previous state set by consecutive calls to transitionTo() or setCurrentState().
   */
  const TId& previousStateId() const {
    assert(mPreviousState.isSet()
               && "No previous state. Check if transitionTo() or setCurrentState() were called more than once");
    return *mPreviousState.id;
  }

  /**
   * The previous state of the FSM.
   * @return Previous state of the FSM.
   * @attention The FSM must have a previous state set by consecutive calls to transitionTo() or setCurrentState().
   */
  const TState& previousState() const {
    assert(mPreviousState.isSet()
               && "No previous state. Check if transitionTo() or setCurrentState() were called more than once");
    return *mPreviousState.state;
  }

  /**
   * The previous state of the FSM.
   * @return Previous state of the FSM.
   * @attention The FSM must have a previous state set by consecutive calls to transitionTo() or setCurrentState().
   */
  TState& previousState() {
    assert(mPreviousState.isSet()
               && "No previous state. Check if transitionTo() or setCurrentState() were called more than once");
    return *mPreviousState.state;
  }

  /**
   * Lists the ids of all states in the FSM.
   * @return List of states ids in the FSM.
   */
  std::vector<TId> statesIds() const {
    std::vector<TId> ids;
    ids.reserve(mStates.size());

    for (const auto& stateMap : mStates) {
      ids.emplace_back(stateMap);
    }

    return ids;
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
