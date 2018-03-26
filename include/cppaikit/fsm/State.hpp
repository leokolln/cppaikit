#pragma once

namespace aikit::fsm {

/**
 * The base class for the state of a Finite State Machine.
 * All state to be used on a FSM should inherit from this class.
 * @tparam TUpdateData Type for the data being passed on update() and fsm::State::update(). Defaults to int.
 * @note The template parameter \a TUpdateData must be the same as the one declared for the FSM.
 * @sa fsm::FSM
 */
template<typename TUpdateData = int>
class State {
 public:
  typedef TUpdateData UpdateData_type;

  virtual ~State() = default;

  /**
   * Method called when a FSM transition to this state.
   * @sa fsm::FSM::transitionTo()
   */
  virtual void onEnter() {};
  /**
   * Method called when a FSM transition from this state.
   * @sa fsm::FSM::transitionTo()
   */
  virtual void onExit() {};
  /**
   * Update the state. This method is called by the FSM when it is updated.
   * @param updateData The data to be used during the update.
   * @sa fsm::FSM::update()
   */
  virtual void update(TUpdateData updateData) = 0;
};

}
