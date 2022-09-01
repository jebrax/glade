#pragma once

class Context;

class State
{
  public:
    State(): suspended(false) {}

    virtual ~State() {}
    virtual void init(Context &context) {}
    virtual void shutdown(Context &context) {}
    virtual void suspend(Context &context) { suspended = true; }
    virtual void wakeup(Context &context) { suspended = false; }
    virtual void applyRules(Context &context) {}

    virtual bool isSuspended() { return suspended; }

  private:
    bool suspended;
};
