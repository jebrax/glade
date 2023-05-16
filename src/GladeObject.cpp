#include <glade/GladeObject.h>
#include <glade/debug/log.h>
#include <glade/exception/GladeException.h>
#include <strings.h>

unsigned int GladeObject::nextObjectId = 0;
std::unordered_map<unsigned int, GladeObject*> GladeObject::indexById;

const unsigned int GladeObject::unusedEntityId = 0;

GladeObject::GladeObject(const unsigned int *force_id):
  enabled(true),
  physicalObject(NULL),
  behavior(NULL),
  physicalObjectEnabled(true),
  behaviorEnabled(true),
  mReplicasOutdated(true),
  name("Undefined")
{
  if (force_id)
    id = *force_id;
  else
    id = ++nextObjectId;

  if (id != unusedEntityId) {
    auto outcome = indexById.insert(std::pair<unsigned int, GladeObject*>(id, this));
    assert(outcome.second && "Object ID clash occured (probably non-thread safe instantiations were made)");
  }

  log("Creating object %d", id);
}

Drawable* GladeObject::getView()
{
  int size = drawables.size();
  if (size == 0)
    return nullptr;
  if (size > 1)
    log("Warning: GladeObject::getView() will return first drawable, but there are more in this entity");

  return drawables[0];
}

void GladeObject::setEnabled(bool enable)
{
  enabled = enable;
}

bool GladeObject::isEnabled(void) const
{
  return enabled;
}

Behavior* GladeObject::getBehavior(void) {
  return behavior;
}

void GladeObject::setBehavior(Behavior *behavior) {
  this->behavior = behavior;
}

void GladeObject::setTransform(Transform &transform)
{
  this->transform = transform;
}

Transform* GladeObject::getTransform(void)
{
  return &transform;
}

void GladeObject::addDrawable(Drawable* view)
{
//  log("Adding drawable");
  drawables.push_back(view);
  visibilitySwitches[view] = true;
}

void GladeObject::addDrawables(Drawables &drawables)
{
  for (DrawablesI di = drawables.begin(); di != drawables.end(); ++di) {
    addDrawable(*di);
  }
}

GladeObject::Drawables* GladeObject::getDrawables(void)
{
  return &drawables;
}

void GladeObject::setPhysicalObject(PhysicalObject &physicalObject)
{
  this->physicalObject = &physicalObject;
}

PhysicalObject* GladeObject::getPhysicalObject()
{
  return physicalObject;
}

void GladeObject::toggleView(Drawable& view, bool enable)
{
  std::map<Drawable*,bool>::iterator i = visibilitySwitches.find(&view);

  if (i != visibilitySwitches.end()) {
    i->second = enable;
  }
}

void GladeObject::toggleView(bool enable)
{
  for (DrawablesI i = drawables.begin(); i != drawables.end(); i++) {
    toggleView(**i, enable);
  }
}

void GladeObject::toggleBehavior(bool enable)
{
  behaviorEnabled = enable;
}

void GladeObject::togglePhysicalObject(bool enable)
{
  physicalObjectEnabled = enable;
}

bool GladeObject::isViewEnabled(Drawable *view) const
{
  std::map<Drawable*,bool>::const_iterator i = visibilitySwitches.find(view);

  if (i != visibilitySwitches.end()) {
    return i->second;
  }

  return false;
}

bool GladeObject::isBehaviorEnabled() const
{
  return behaviorEnabled;
}

bool GladeObject::isPhysicalObjectEnabled() const
{
  return physicalObjectEnabled;
}

/** This is for debugging purposes */
const std::string* GladeObject::getName(void) const
{
  return &name;
}

/** This is for debugging purposes */
void GladeObject::setName(const std::string &name)
{
  this->name = name;
}