#pragma once

#include <glade/controls/KeyMapper.h>
#include <glade/controls/Key.h>
#include <unordered_map>

class VirtualController {
public:
  VirtualController() : mKeyMapper(nullptr) {}

  virtual ~VirtualController() {
    if (mKeyMapper) delete mKeyMapper;
  }

  // takes ownership
  void setKeyMapper(Glade::KeyMapper *key_mapper);
  bool updateKeyState(Glade::Key control_id, int terminal_id, bool is_pressed);
  bool updateInputActionState(unsigned int input_action, bool is_pressed);
  bool isActionNetworked(unsigned int input_action);
  virtual bool isKeyPressed(Glade::Key control_id);
  virtual bool isActionPressed(unsigned int input_action);
  virtual bool inputActionPress(unsigned int control_id, int terminal_id) = 0;
  virtual bool inputActionRelease(unsigned int control_id, int terminal_id) = 0;

  virtual bool pointerDown(float axis_x, float axis_y, float axis_z, int control_id,
                           int terminal_id) = 0;

  virtual bool pointerUp(float axis_x, float axis_y, float axis_z, int control_id,
                         int terminal_id) = 0;

  virtual bool pointerMove(float axis_x, float axis_y, float axis_z, int control_id,
                           int terminal_id, bool is_absolute = true) = 0;

private:
  Glade::KeyMapper *mKeyMapper;
  std::unordered_map<Glade::Key, bool> mKeysState;
  std::unordered_map<unsigned int, bool> mInputActionsState;
};
