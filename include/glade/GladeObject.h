#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <optional>

#include <glade/collisions/Collider.h>
#include <glade/math/Transform.h>
#include <glade/debug/log.h>
#include <glade/io/Stream.h>

class Behavior;
class PhysicalObject;
class Drawable;

class GladeObject {
public:
  typedef std::vector<Drawable *> Drawables;
  typedef Drawables::iterator DrawablesI;

  // not thread safe
  static unsigned int nextObjectId;
  static std::unordered_map<unsigned int, GladeObject *> indexById;

  static const unsigned int UNUSED_ENTITY_ID;

  class Comparator {
  public:
    virtual int compare(GladeObject &first, GladeObject &second) = 0;
    virtual ~Comparator() {};
  };

protected:
  bool enabled;
  PhysicalObject *physicalObject;
  std::optional<Collider> collider;
  Behavior *behavior;
  Drawables drawables;
  std::map<Drawable *, bool> visibilitySwitches;
  bool physicalObjectEnabled, behaviorEnabled;
  bool initialized = false;

private:
  std::string name;
  unsigned int id;
  Transform transform;
  bool mReplicasOutdated;

public:
  GladeObject(const unsigned int *force_id);
  virtual void setEnabled(bool enable);
  virtual bool isEnabled(void) const;
  virtual Behavior *getBehavior(void);
  virtual void setBehavior(Behavior *behavior_);
  virtual void setTransform(Transform &transform_);
  virtual Transform *getTransform(void);
  virtual void addDrawable(Drawable *view);
  virtual void addDrawables(Drawables &drawables_);
  virtual Drawables *getDrawables(void);
  virtual void setPhysicalObject(PhysicalObject &physical_object);
  virtual PhysicalObject *getPhysicalObject();

  // Shape must outlive this collider. Remove the existing collider from its
  // CollisionWorld before replacing or clearing it, or destroying this object.
  Collider &setCollider(const CollisionShape &shape, Collider::Type type = Collider::Type::STATIC);

  Collider *getCollider();

  const Collider *getCollider() const;

  void clearCollider();

  virtual void toggleView(Drawable &view, bool enable);
  virtual void toggleView(bool enable);
  virtual void toggleBehavior(bool enable);
  virtual void togglePhysicalObject(bool enable);
  virtual bool isViewEnabled(Drawable *view) const;
  virtual bool isBehaviorEnabled() const;
  virtual bool isPhysicalObjectEnabled() const;

  virtual void serialize(Glade::MemoryOutputStream &stream) {
    stream.write(id);
    stream.write(getType());
    stream.write(transform.position->x);
    stream.write(transform.position->y);
    stream.write(transform.position->z);
    stream.write(transform.rotation->x);
    stream.write(transform.rotation->y);
    stream.write(transform.rotation->z);
  }

  virtual void unserialize(Glade::MemoryInputStream &stream) {
    stream.read(transform.position->x);
    stream.read(transform.position->y);
    stream.read(transform.position->z);
    stream.read(transform.rotation->x);
    stream.read(transform.rotation->y);
    stream.read(transform.rotation->z);
  }

  virtual void setReplicasOutdated(bool outdated) { mReplicasOutdated = outdated; }
  virtual bool areReplicasOutdated() { return mReplicasOutdated; }
  virtual unsigned int getType() { return 0; }
  unsigned int getId() { return id; }
  // returns first element of the drawables array (or nullptr)
  Drawable *getView();

  /** This is for debugging purposes */
  virtual const std::string *getName(void) const;
  virtual void setName(const std::string &name_);
};
