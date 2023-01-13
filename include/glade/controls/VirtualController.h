#pragma once

#include <glade/controls/KeyMapper.h>
#include <glade/controls/Key.h>
#include <unordered_map>

class VirtualController
{
  public:
    VirtualController(): mKeyMapper(nullptr) {}

    // takes ownership
    void SetKeyMapper(Glade::KeyMapper *keyMapper)
    {
      if (mKeyMapper)
        delete mKeyMapper;

      mKeyMapper = keyMapper;
    }

    virtual ~VirtualController()
    {
      if (mKeyMapper)
        delete mKeyMapper;
    }

    virtual bool isKeyPressed(Glade::Key controlId) {
      auto keyState = mKeysState.find(controlId);
      return (keyState != mKeysState.end() && keyState->second);
    }

    virtual bool isActionPressed(unsigned int inputAction) {
      auto inputActionState = mInputActionsState.find(inputAction);
      return (inputActionState != mInputActionsState.end() && inputActionState->second);
    }

    bool UpdateKeyState(Glade::Key controlId, int terminalId, bool isPressed)
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

    bool UpdateInputActionState(unsigned int inputAction, bool isPressed)
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

    virtual bool InputActionPress(unsigned int controlId, int terminalId) = 0;
    virtual bool InputActionRelease(unsigned int controlId, int terminalId) = 0;
    virtual bool pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId) = 0;
    virtual bool pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId) = 0;
    virtual bool pointerMove(float axisX, float axisY, float axisZ, int controlId, int terminalId, bool isAbsolute = true) = 0;

private:
  Glade::KeyMapper* mKeyMapper;
  std::unordered_map<Glade::Key, bool> mKeysState;
  std::unordered_map<unsigned int, bool> mInputActionsState;
};
