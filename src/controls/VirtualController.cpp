#include <cassert>
#include <glade/controls/VirtualController.h>
#include <glade/controls/definitions.h>

#include <unordered_map>

void VirtualController::SetKeyMapper(Glade::KeyMapper *keyMapper)
{
  if (mKeyMapper)
    delete mKeyMapper;

  mKeyMapper = keyMapper;
}

bool VirtualController::isKeyPressed(Glade::Key controlId) {
  auto keyState = mKeysState.find(controlId);
  return (keyState != mKeysState.end() && keyState->second);
}

bool VirtualController::isActionPressed(unsigned int inputAction) {
  auto inputActionState = mInputActionsState.find(inputAction);
  return (inputActionState != mInputActionsState.end() && inputActionState->second);
}

bool VirtualController::IsActionNetworked(unsigned int inputAction) {
  return (inputAction > GLADE_CONTROLS_MAX_LOCAL_ACTION_ID);
}

bool VirtualController::UpdateKeyState(Glade::Key controlId, int terminalId, bool isPressed)
{
  auto keyState = mKeysState.find(controlId);
  bool changedState = true;

  if (keyState == mKeysState.end()) {
    mKeysState[controlId] = isPressed;
  } else {
    changedState = (keyState->second != isPressed);
    keyState->second = isPressed;
  }

  if (changedState) {
    assert(mKeyMapper);
    unsigned int inputAction = mKeyMapper->GetMappedAction(controlId);
    UpdateInputActionState(inputAction, isPressed);
  }

  return changedState;
}

bool VirtualController::UpdateInputActionState(unsigned int inputAction, bool isPressed)
{
  auto inputActionState = mInputActionsState.find(inputAction);
  bool changedState = true;

  if (inputActionState == mInputActionsState.end()) {
    mInputActionsState[inputAction] = isPressed;
  } else {
    changedState = (inputActionState->second != isPressed);
    inputActionState->second = isPressed;
  }

  if (changedState) {
    if (isPressed)
      InputActionPress(inputAction, 0);
    else
      InputActionRelease(inputAction, 0);
  }

  return changedState;
}
