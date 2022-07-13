#include <glade/GladeObject.h>
#include <glade/debug/log.h>
#include <glade/exception/GladeException.h>

GladeObject::GladeObject(void):
  enabled(true),
  physicBody(NULL),
  collisionShape(NULL),
  behavior(NULL),
  colShapeEnabled(true),
  physicBodyEnabled(true),
  behaviorEnabled(true),
  name("Undefined")
{
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

void GladeObject::setPhysicBody(PhysicBody &physicBody)
{
  this->physicBody = &physicBody;
}

PhysicBody* GladeObject::getPhysicBody()
{
  return physicBody;
}

CollisionShape* GladeObject::getCollisionShape(void) {
  return collisionShape;
}

void GladeObject::setCollisionShape(CollisionShape &collisionShape) {
  this->collisionShape = &collisionShape;
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

void GladeObject::toggleCollisionShape(bool enable)
{
  colShapeEnabled = enable;
}

void GladeObject::toggleBehavior(bool enable)
{
  behaviorEnabled = enable;
}

void GladeObject::togglePhysicBody(bool enable)
{
  physicBodyEnabled = enable;
}

bool GladeObject::isViewEnabled(Drawable *view) const
{
  std::map<Drawable*,bool>::const_iterator i = visibilitySwitches.find(view);

  if (i != visibilitySwitches.end()) {
    return i->second;
  }

  return false;
}

bool GladeObject::isCollisionShapeEnabled() const
{
  return colShapeEnabled;
}


bool GladeObject::isBehaviorEnabled() const
{
  return behaviorEnabled;
}


bool GladeObject::isPhysicBodyEnabled() const
{
  return physicBodyEnabled;
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