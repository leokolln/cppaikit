#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <vector>

#include "State.hpp"

namespace aikit::fsm {

// TODO Implement previous state and associated methods
// TODO Create (or rename) a method to initial state?

/**
 * Implementation for a Finite State Machine.
 * @tparam TId Type for the id of a state. Defaults to std::string.
 * @tparam TUpdateData Type for the data being passed on update() and fsm::State::update(). Defaults to int.
 * @sa fsm::State
 */
template<typename TId = std::string, typename TUpdateData = int>
class FSM {
 public:
  typedef TId Id_type;
  typedef TUpdateData UpdateData_type;

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
    static_assert(std::is_base_of_v<State<TUpdateData>, TNewStateNoRef>,
                  "addState() is only callable with state that is derived from fsm::State<TUpdateData>");

    mStates.try_emplace(std::move(id), std::make_unique<TNewStateNoRef>(std::forward<TNewState>(state)));
  }

  /**
   * Remove a state from the FSM.
   * @param id Identification of the state being removed.
   * @return Returns true only when a state with associated \a id is found and consequentially removed.
   */
  bool removeState(const TId& id) {
    if (currentStateId() == id) {
      mCurrentStateId = nullptr;
      mCurrentState = nullptr;
      // TODO Set current state as previous state if there is one
    }

    return mStates.erase(id) > 0;
    // FIXME What happens when the state removed is the previous state?
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

    if (mCurrentState != nullptr) {
      mCurrentState->onExit();
    }

    mCurrentStateId = &(found->first);
    mCurrentState = found->second.get();
    mCurrentState->onEnter();
  }

  /**
   * Update FSM and it's current state.
   * @param updateData The data that will be passed during the call fsm::State::update() on the current state.
   * @attention The FSM must have a current state that was previously set by either transitionTo() or setCurrentState().
   * @sa fsm::State::update()
   */
  void update(TUpdateData updateData) {
    assert(mCurrentState != nullptr
               && "The FSM has no current state. Check if transitionTo() or setCurrentState() were called");
    mCurrentState->update(updateData);
  }

  /**
   * Set the current state of the FSM.
   * This method should be preferably used to define the initial state of the machine or in any other circumstance
   * where calls to fsm::State::onExit() and fsm::State::onEnter() are undesirable.
   * @param id Identification of the state that will be set as current.
   * @note This method can be called even if no previous state was set as current.
   * @attention fsm::State::onExit() will not be called for the current state (if any).
   * @attention fsm::State::onEnter() will not be called for the state being set as current.
   * @attention It is required that a state with the associated \a id is found on the FSM.
   */
  void setCurrentState(const TId& id) {
    auto found = mStates.find(id);
    assert(found != mStates.end()
               && "No state with informed id was found on the FSM. Check if it was added or if id is incorrect");

    mCurrentStateId = &(found->first);
    mCurrentState = found->second.get();
  }

  /**
   * The identification of the current state of the FSM.
   * @return Id of the current state.
   * @attention The FSM must have a current state that was previously set by either transitionTo() or setCurrentState().
   */
  const TId& currentStateId() const {
    assert(mCurrentStateId != nullptr
               && "The FSM has no current state. Check if transitionTo() or setCurrentState() were called");
    return *mCurrentStateId;
  }

  /**
   * The current state of the FSM.
   * @return Current state of the FSM.
   * @attention The FSM must have a current state that was previously set by either transitionTo() or setCurrentState().
   */
  const State<TUpdateData>& currentState() const {
    assert(mCurrentState != nullptr
               && "The FSM has no current state. Check if transitionTo() or setCurrentState() were called");
    return *mCurrentState;
  }

  /**
   * Check if the FSM has a current state set.
   * @return True if there is a current state.
   */
  bool hasCurrentState() const {
    return mCurrentState != nullptr;
  }

  /**
   * The current state of the FSM.
   * @return Current state of the FSM.
   * @attention The FSM must have a current state that was previously set by either transitionTo() or setCurrentState().
   */
  State<TUpdateData>& currentState() {
    assert(mCurrentState != nullptr
               && "The FSM has no current state. Check if transitionTo() or setCurrentState() were called");
    return *mCurrentState;
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
  std::map<TId, std::unique_ptr<State<TUpdateData>>> mStates; ///< Mapping of states and associated ids.
  const TId* mCurrentStateId = nullptr; ///< The id of the FSM's current state.
  State<TUpdateData>* mCurrentState = nullptr; ///< The FSM's current state.
};

}
