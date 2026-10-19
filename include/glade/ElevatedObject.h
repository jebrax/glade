#pragma once

#include <glade/GladeObject.h>

// X/Y remain horizontal position and ground depth. Elevation and height are
// world-unit physical values, independent of the inherited render Z.
class ElevatedObject : public GladeObject {
public:
  explicit ElevatedObject(float height_ = 0.0f);
  virtual ~ElevatedObject() = default;

  ElevatedObject(const ElevatedObject &) = delete;
  ElevatedObject &operator=(const ElevatedObject &) = delete;

  float getElevation() const;

  // Negative elevations are silently clamped to zero for now.
  void setElevation(float elevation_);

  float getHeight() const;

  float getTopElevation() const;

  bool hasSupportingSurface() const { return supportingSurface; }
  void setSupportingSurface(bool supporting_surface) { supportingSurface = supporting_surface; }

  bool isGrounded() const { return grounded; }

protected:
  // Called after elevation changes; never called from the base constructor.
  virtual void updateElevation() = 0;

private:
  friend class KinematicMover;

  void setGrounded(bool grounded_) { grounded = grounded_; }

  bool supportingSurface = false;
  bool grounded = true;
  float elevation = 0.0f;
  const float height;
};
