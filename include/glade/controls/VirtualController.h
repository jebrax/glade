#pragma once

#include <glade/controls/KeyMapper.h>
#include <glade/controls/Key.h>
#include <unordered_map>

class VirtualController
{
  public:
    VirtualController(): mKeyMapper(nullptr) {}

    virtual ~VirtualController()
    {
      if (mKeyMapper)
        delete mKeyMapper;
    }

    // takes ownership
    void SetKeyMapper(Glade::KeyMapper *keyMapper);
    bool UpdateKeyState(Glade::Key controlId, int terminalId, bool isPressed);
    bool UpdateInputActionState(unsigned int inputAction, bool isPressed);
    bool IsActionNetworked(unsigned int inputAction);
    virtual bool isKeyPressed(Glade::Key controlId);
    virtual bool isActionPressed(unsigned int inputAction);

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
