#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include <glade/math/Transform.h>
#include <glade/debug/log.h>
#include <glade/io/Stream.h>

class Behavior;
class PhysicalObject;
class Drawable;

class GladeObject {
public:
  typedef std::vector<Drawable*> Drawables;
  typedef Drawables::iterator    DrawablesI;

  // not thread safe
  static unsigned int nextObjectId;
  static std::unordered_map<unsigned int, GladeObject*> indexById;

  static const unsigned int unusedEntityId;

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
  unsigned int id;
  Transform transform;

public:
  GladeObject(const unsigned int *force_id);
  virtual void setEnabled(bool enable);
  virtual bool isEnabled(void) const;
  virtual Behavior* getBehavior(void);
  virtual void setBehavior(Behavior *behavior);
  virtual void setTransform(Transform &transform);
  virtual Transform* getTransform(void);
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

  virtual void serialize(Glade::MemoryOutputStream &stream) {
    stream.Write(id);
    stream.Write(getType());
    stream.Write(transform.position->x);
    stream.Write(transform.position->y);
    stream.Write(transform.position->z);
    stream.Write(transform.rotation->x);
    stream.Write(transform.rotation->y);
    stream.Write(transform.rotation->z);
  }

  virtual void unserialize(Glade::MemoryInputStream &stream) {
    stream.Read(transform.position->x);
    stream.Read(transform.position->y);
    stream.Read(transform.position->z);
    stream.Read(transform.rotation->x);
    stream.Read(transform.rotation->y);
    stream.Read(transform.rotation->z);
  }

  virtual unsigned int getType() { return 0; }
  unsigned int getId() { return id; }
  // returns first element of the drawables array (or nullptr)
  Drawable *getView();

  /** This is for debugging purposes */
  virtual const std::string* getName(void) const;
  virtual void setName(const std::string &name);
};
