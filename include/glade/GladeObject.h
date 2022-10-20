#pragma once

#include <string>
#include <vector>
#include <map>

#include "math/Transform.h"

class Behavior;
class PhysicalObject;
class Drawable;

class GladeObject {
public:
  typedef std::vector<Drawable*>    Drawables;
  typedef Drawables::iterator    DrawablesI;

  class Comparator
  {
    public:
      virtual int compare(GladeObject &first, GladeObject &second) = 0;
      virtual ~Comparator() {};
  };

protected:
  bool enabled;
  PhysicalObject* physicalObject;
  Behavior* behavior;
  Drawables drawables;
  std::map<Drawable*, bool> visibilitySwitches;
  bool physicalObjectEnabled, behaviorEnabled;
  bool initialized = false;

private:
  std::string name;
  Transform transform;

public:
  GladeObject(void);
  virtual void setEnabled(bool enable);
  virtual bool isEnabled(void) const;
  virtual Behavior* getBehavior(void);
  virtual void setBehavior(Behavior *behavior);
  virtual void setTransform(Transform &transform);
  virtual Transform* getTransform(void);
  // FIXME should be unique_ptr<Drawable>
  virtual void addDrawable(Drawable* view);
  virtual void addDrawables(Drawables &drawables);
  virtual Drawables* getDrawables(void);

  virtual void setPhysicalObject(PhysicalObject &physicalObject);
  virtual PhysicalObject* getPhysicalObject();
  virtual void toggleView(Drawable& view, bool enable);
  virtual void toggleView(bool enable);
  virtual void toggleBehavior(bool enable);
  virtual void togglePhysicalObject(bool enable);
  virtual bool isViewEnabled(Drawable *view) const;
  virtual bool isBehaviorEnabled() const;
  virtual bool isPhysicalObjectEnabled() const;

  // returns first element of the drawables array (or nullptr)
  Drawable *getView();

  /** This is for debugging purposes */
  virtual const std::string* getName(void) const;
  virtual void setName(const std::string &name);
};
