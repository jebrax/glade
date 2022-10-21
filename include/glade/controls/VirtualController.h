#pragma once

#include <unordered_map>

namespace Glade {

enum Key {
  GLADE_KEY_W,
  GLADE_KEY_S,
  GLADE_KEY_A,
  GLADE_KEY_D,
  GLADE_KEY_SPACE,
  GLADE_KEY_X,
  GLADE_KEY_G,
  GLADE_KEY_F,
  GLADE_KEY_T,
  GLADE_KEY_R,
  GLADE_KEY_C,
  GLADE_KEY_Q,
  GLADE_KEY_E,
  GLADE_KEY_LEFT,
  GLADE_KEY_RIGHT,
  GLADE_KEY_UP,
  GLADE_KEY_DOWN,
  GLADE_KEY_O,
  GLADE_KEY_L,
  GLADE_KEY_1,
  GLADE_KEY_2,
  GLADE_KEY_3,
  GLADE_KEY_4,
  GLADE_KEY_5,
};

}

class VirtualController
{
  std::unordered_map<Glade::Key, bool> keysState;

  public:
    virtual void initController() {};

    virtual bool isKeyPressed(Glade::Key controlId) {
      auto keyState = keysState.find(controlId);
      return (keyState != keysState.end() && keyState->second);
    }

    virtual bool buttonPress(Glade::Key controlId, int terminalId) {
      auto keyState = keysState.find(controlId);
      bool changedState = true;

      if (keyState == keysState.end()) {
        keysState[controlId] = true;
      } else {
        changedState = !keyState->second;
        keyState->second = true;
      }

      return changedState;
    }

    virtual bool buttonRelease(Glade::Key controlId, int terminalId) {
      auto keyState = keysState.find(controlId);
      bool changedState = true;

      if (keyState == keysState.end()) {
        keysState[controlId] = false;
      } else {
        changedState = keyState->second;
        keyState->second = false;
      }

      return changedState;
    }

    virtual bool pointerDown(float axisX, float axisY, float axisZ, int controlId, int terminalId) = 0;
    virtual bool pointerUp(float axisX, float axisY, float axisZ, int controlId, int terminalId) = 0;
    virtual bool pointerMove(float axisX, float axisY, float axisZ, int controlId, int terminalId, bool isAbsolute = true) = 0;
};
