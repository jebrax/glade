#pragma once

class Context;
class GladeObject;

class State {
public:
  State() : suspended(false) {}
  virtual ~State() {}
  virtual void init(Context &context) {}
  virtual void shutdown(Context &context) {}
  virtual void suspend(Context &context) { suspended = true; }
  virtual void wakeup(Context &context) { suspended = false; }
  // Per-frame rules, called once per normal Context update.
  virtual void update(Context &context) {}
  // Fixed simulation interval in seconds. Context may call this zero or more times per frame.
  virtual void fixedUpdate(Context &context, float seconds) {}
  virtual bool isSuspended() { return suspended; }

  virtual GladeObject *createEntityByTypeId(unsigned int type,
                                          const unsigned int *force_id = nullptr,
                                          bool replicate = true) {
    return nullptr;
  }

  virtual void onClientInputReceived(unsigned int user_id, unsigned int glade_key_code,
                                     bool is_pressed) {}

  virtual void onEntityReplicated(GladeObject *entity) {}
  virtual void onEntityUnserialized(GladeObject *entity) {}
  virtual void onUserLoggedIn(unsigned int user_id) {}

private:
  bool suspended;
};
