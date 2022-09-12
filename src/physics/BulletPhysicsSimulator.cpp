#include <glade/physics/Simulator.h>
#include <glade/physics/PhysicBody.h>
#include <glade/GladeObject.h>
#include <glade/debug/log.h>

void Simulator::clear()
{
  dynamicObjects.clear();
  paused = false;
  gravity.set(0, 0, 0);
}

void Simulator::add(GladeObject* object)
{
  if (object->getPhysicBody() != NULL) {
    dynamicObjects.insert(object);
  }
}

void Simulator::remove(GladeObject* object)
{
  dynamicObjects.erase(object);
}

void Simulator::stepSimulation(double deltaTime)
{
  if (paused) {
    return;
  }

  std::set<GladeObject*>::iterator i = dynamicObjects.begin();

  while (i != dynamicObjects.end()) {
    PhysicBody *body = (*i)->getPhysicBody();
    Transform *transform = (*i)->getTransform();

    if (body != NULL && (*i)->isPhysicBodyEnabled()) {
      // calculating velocity
      if (body->acceleration.x) {
        body->velocity.x += deltaTime * body->acceleration.x;
      } else if (body->velocity.x) {
        int sign = body->velocity.x > 0 ? 1 : -1;
        body->velocity.x += -sign * body->slowDown.x;
        int newSign = body->velocity.x > 0 ? 1 : -1;
        
        if (sign != newSign || abs(body->velocity.x) < body->minSpeed.x) {
          body->velocity.x = 0;
        }
      }
      
      if (body->acceleration.y) {
        body->velocity.y += deltaTime * body->acceleration.y;
      } else if (body->velocity.y) {
        int sign = body->velocity.y > 0 ? 1 : -1;
        body->velocity.y += -sign * body->slowDown.y;
        int newSign = body->velocity.y > 0 ? 1 : -1;
        
        if (sign != newSign || abs(body->velocity.y) < body->minSpeed.y) {
          body->velocity.y = 0;
        }
      }
      
      if (body->acceleration.z) {
        body->velocity.z += deltaTime * body->acceleration.z;
      } else if (body->velocity.z) {
        int sign = body->velocity.z > 0 ? 1 : -1;
        body->velocity.z += -sign * body->slowDown.z;
        int newSign = body->velocity.z > 0 ? 1 : -1;
        
        if (sign != newSign || abs(body->velocity.z) < body->minSpeed.z) {
          body->velocity.z = 0;
        }
      }
 
       // constraining velocity
      if (abs(body->velocity.x) > body->maxSpeed.x) {
        body->velocity.x = (body->velocity.x > 0 ? 1 : -1) * body->maxSpeed.x;
      }
      
      if (abs(body->velocity.y) > body->maxSpeed.y) {
        body->velocity.y = (body->velocity.y > 0 ? 1 : -1) * body->maxSpeed.y;
      }
      
      if (abs(body->velocity.z) > body->maxSpeed.z) {
        body->velocity.z = (body->velocity.z > 0 ? 1 : -1) * body->maxSpeed.z;
      }
      
      // updating coords
      transform->getPosition()->x += deltaTime * body->velocity.x;
      transform->getPosition()->y += deltaTime * body->velocity.y;
      transform->getPosition()->z += deltaTime * body->velocity.z;
    }
    
    ++i;
  }
}