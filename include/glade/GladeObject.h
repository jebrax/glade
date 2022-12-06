#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include <glade/math/Transform.h>
#include <glade/debug/log.h>

class Behavior;
class PhysicalObject;
class Drawable;

class GladeObject {
public:
  typedef std::vector<Drawable*>    Drawables;
  typedef Drawables::iterator    DrawablesI;

  // not thread safe
  static unsigned int nextObjectId;
  static std::unordered_map<unsigned int, GladeObject*> indexById;

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

  virtual int serialize(unsigned char *buffer, int buffer_size) {
    unsigned char size = sizeof(float) * 6 + sizeof(unsigned int) + sizeof(unsigned char);

    if (buffer_size < size)
      return size;

    unsigned char *ptr = buffer;
    ptr[0] = size;
    ptr++;

    unsigned int *ptrint = (unsigned int*) ptr;
    *ptrint = id;
    ptrint++;

    float *ptrfloat = (float*) ptrint;

    *ptrfloat = transform.position->x;
    ptrfloat++;
    *ptrfloat = transform.position->y;
    ptrfloat++;
    *ptrfloat = transform.position->z;
    ptrfloat++;

    *ptrfloat = transform.rotation->x;
    ptrfloat++;
    *ptrfloat = transform.rotation->y;
    ptrfloat++;
    *ptrfloat = transform.rotation->z;

    return size;
  }

  static void unsearialize(unsigned char *buffer) {
    unsigned char *ptr = buffer;
    char buffer_size = ptr[0];
    ptr++;

    unsigned char size = sizeof(float) * 6 + sizeof(unsigned int) + sizeof(unsigned char);
    if (buffer_size < size) {
      log("Warning: couldn't unserialize object, unexpected packet size");
      return;
    }

    unsigned int *ptrint = (unsigned int*) ptr;
    unsigned int receivedId = *ptrint;
    ptrint++;

    auto objecti = indexById.find(receivedId);

    // TODO instantiate object of a proper type
    assert(objecti != indexById.end() && "Failed to unserialize object: object not found");
    GladeObject* obj = objecti->second;

    float *ptrfloat = (float*) ptrint;
    obj->transform.position->x = *ptrfloat;
    ptrfloat++;
    obj->transform.position->y = *ptrfloat;
    ptrfloat++;
    obj->transform.position->z = *ptrfloat;
    ptrfloat++;

    obj->transform.rotation->x = *ptrfloat;
    ptrfloat++;
    obj->transform.rotation->y = *ptrfloat;
    ptrfloat++;
    obj->transform.rotation->z = *ptrfloat;
  }

  unsigned int getId() { return id; }
  // returns first element of the drawables array (or nullptr)
  Drawable *getView();

  /** This is for debugging purposes */
  virtual const std::string* getName(void) const;
  virtual void setName(const std::string &name);
};
