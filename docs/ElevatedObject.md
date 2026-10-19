# ElevatedObject

`ElevatedObject` derives from `GladeObject` and stores elevation (initially zero)
and immutable height (default zero). Height must be finite and nonnegative.
Top elevation is computed as elevation + height. Height zero can represent an
object whose vertical extent is not modelled yet; it does not imply support.

`setElevation` clamps finite negative inputs to zero and invokes the derived
`updateElevation()` hook. Nonfinite inputs or a nonfinite resulting top elevation
are rejected before state changes. The constructor does not invoke the hook.
The inherited transform is unchanged: X/Y remain ground coordinates, while
render Z is chosen by the application. Derived classes control visual projection.

`hasSupportingSurface()` / `setSupportingSurface()` describe whether the top
can establish grounded state. The flag defaults to false. Height or elevation
alone does not make a surface supporting. `isGrounded()` reports support; only
the mover sets raised support. Direct `setElevation()` is placement: it sets
grounded only at zero, with raised support refreshed on the next mover call.

`MoveRequest::elevationDisplacement` requests physical height movement separately
from render Z. KinematicMover resolves ascent, X/Y movement, then descent at the
resolved X/Y, and refreshes grounded even for zero requests and overlap recovery.
This is staged movement, not a simultaneous 3D swept-volume solver. Results
include requested/actual elevation displacement. Rising bodies stop at solid
undersides; descending bodies stop at solid tops. Only opted-in tops establish
grounded. A non-supporting solid can block descent without supplying grounded.
Support requires positive footprint overlap; touching an edge alone is insufficient.

Finite-height elevated obstacles block X/Y only when their vertical extent
intersects the body's extent. Height-zero moving bodies use their feet elevation.
Zero-height non-supporting terrain and non-elevated owners retain legacy wall
behaviour. A zero-height supporting object represents a horizontal surface with
no side volume. Queries remain X/Y queries; the mover supplies a predicate to
filter side blockers before selecting the earliest hit. Support/descent queries
use the actual shape footprint, not just broadphase bounds. Category masks,
sensors and enabled state apply to both collision and support.

The reference floor remains elevation zero. Leaving raised support clears
grounded without changing elevation: gravity and jumping are game rules and
are not added here. No support pointer is stored, so removal cannot leave a
dangling surface reference; state is refreshed on the next move.

GladeObject serialization is unchanged. Registered colliders must still be
removed before their shape or owner is destroyed, and derived classes retain
object-registry cleanup responsibility. Destruction through ElevatedObject is
virtual; GladeObject still has a nonvirtual destructor.

`tests/ElevatedObjectTest.cpp` checks the data/state contract.
`tests/ElevatedMovementTest.cpp` checks filtering, landings, repeated descent,
edge overlap, support loss, raised bases, underside blocking, highest support,
polygon footprints, legacy walls and invalid requests. The tests CMake host
exposes `glade_elevated_object_test` and `glade_elevated_movement_test` when a
`glade` target is supplied.
