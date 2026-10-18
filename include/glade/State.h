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
  virtual void applyRules(Context &context) {}
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
