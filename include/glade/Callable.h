#pragma once

#include <set>
#include <memory>

class Callable {
public:
  typedef std::set<Callable *> Callables;
  typedef Callables::iterator CallablesI;
  typedef std::unique_ptr<Callable> AutoDispose;

  virtual void call() = 0;
  virtual ~Callable() {}
};
