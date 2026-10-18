#include <cassert>
#include <glade/controls/VirtualController.h>
#include <glade/controls/definitions.h>

#include <unordered_map>

void VirtualController::setKeyMapper(Glade::KeyMapper *key_mapper) {
  if (mKeyMapper) delete mKeyMapper;

  mKeyMapper = key_mapper;
}

bool VirtualController::isKeyPressed(Glade::Key control_id) {
  auto key_state = mKeysState.find(control_id);
  return (key_state != mKeysState.end() && key_state->second);
}

bool VirtualController::isActionPressed(unsigned int input_action) {
  auto input_action_state = mInputActionsState.find(input_action);
  return (input_action_state != mInputActionsState.end() && input_action_state->second);
}

bool VirtualController::isActionNetworked(unsigned int input_action) {
  return (input_action > GLADE_CONTROLS_MAX_LOCAL_ACTION_ID);
}

bool VirtualController::updateKeyState(Glade::Key control_id, int terminal_id, bool is_pressed) {
  auto key_state = mKeysState.find(control_id);
  bool changed_state = true;

  if (key_state == mKeysState.end()) {
    mKeysState[control_id] = is_pressed;
  } else {
    changed_state = (key_state->second != is_pressed);
    key_state->second = is_pressed;
  }

  if (changed_state) {
    assert(mKeyMapper);
    unsigned int input_action = mKeyMapper->getMappedAction(control_id);
    updateInputActionState(input_action, is_pressed);
  }

  return changed_state;
}

bool VirtualController::updateInputActionState(unsigned int input_action, bool is_pressed) {
  auto input_action_state = mInputActionsState.find(input_action);
  bool changed_state = true;

  if (input_action_state == mInputActionsState.end()) {
    mInputActionsState[input_action] = is_pressed;
  } else {
    changed_state = (input_action_state->second != is_pressed);
    input_action_state->second = is_pressed;
  }

  if (changed_state) {
    if (is_pressed) inputActionPress(input_action, 0);
    else inputActionRelease(input_action, 0);
  }

  return changed_state;
}
