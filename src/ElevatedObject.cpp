#include <glade/ElevatedObject.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>

ElevatedObject::ElevatedObject(float height_) : GladeObject(nullptr), height(height_) {
  if (!std::isfinite(height) || height < 0.0f) {
    // GladeObject registers during construction and has no registry cleanup.
    indexById.erase(getId());
    throw std::invalid_argument("Height must be finite and nonnegative");
  }
}

float ElevatedObject::getElevation() const { return elevation; }

void ElevatedObject::setElevation(float elevation_) {
  const float next_elevation = std::max(0.0f, elevation_);

  if (!std::isfinite(elevation_) || !std::isfinite(next_elevation + height)) {
    throw std::invalid_argument("Elevation and top elevation must be finite");
  }

  elevation = next_elevation;
  // Direct placement has no collision-world context. The mover refreshes
  // raised support; elevation zero remains the implicit reference floor.
  grounded = elevation == 0.0f;
  updateElevation();
}

float ElevatedObject::getHeight() const { return height; }

float ElevatedObject::getTopElevation() const { return elevation + height; }
