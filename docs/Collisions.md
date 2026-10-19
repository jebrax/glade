# Collision detection and kinematic movement

The `glade/collisions` module provides detection queries independently of the
existing `physics/CollisionDetector`. `CollisionWorld` only answers queries;
`KinematicMover` separately applies constrained movement and overlap recovery.
Neither dispatches events or runs gameplay callbacks. `Context` owns one
CollisionWorld alongside its other systems, without a per-frame world update.

## Geometry and transforms

`RectangleCollisionShape` currently supports axis-aligned rectangles
in physical X/Y. Its constructor takes positive half-width and half-height in
world units. Geometry is centred on its transform's position. Z, rotation and
scale are ignored, including when computing broadphase bounds. Sprite frames
and visual scale do not affect collision dimensions.

`ConvexPolygonCollisionShape` stores an immutable boundary of three or more
local X/Y vertices. Supply boundary order in either winding; the constructor
normalises it to counterclockwise. Vertices must be finite and form a simple,
strictly convex polygon. Concave/self-intersecting boundaries, repeated vertices,
zero area and redundant collinear edges throw `std::invalid_argument`. The
constructor does not compute a convex hull or silently reshape terrain.
The collider origin need not be the polygon's centre. Placement uses translation
only: Z, rotation and scale are ignored just as for rectangles. Polygon bounds
enclose the translated vertices and are rounded outward when necessary.

`CollisionShape` separates geometry from placement. Its type identifies which
narrowphase algorithms apply; adding a shape type requires implementing its
supported overlap and cast pairs. Bounds are conservative candidate geometry,
not a substitute for those algorithms. Rectangle bounds happen to be exact.
All rectangle/polygon query pair directions and polygon/polygon queries support
overlap and translation casts. Rectangle/rectangle queries retain their specialised
implementation. Other pairs use separating axes from both boundaries, with swept
projection intervals for casts. Interval checks use unnormalised axes; reported
normals and penetration distances are normalised. Polygon queries generate one
or two contact points on the encountered boundary's supporting vertex or edge.
They preserve the existing touching and containment contracts below.

`Collider` refers to an owner and a shape, and stores a relative `Transform`.
`getWorldTransform()` uses `Transform::combine` with the owner's transform as
the base. This follows Glade's existing composition rules: positions and
rotations add, and scales multiply. The shape then uses only the supported
transform components.

## Ownership and registration

`GladeObject` optionally owns a `Collider`, created with `setCollider()` and
accessed through `getCollider()`. `clearCollider()` removes the optional value.
The referenced shape must outlive the collider. Colliders cannot be copied or
moved; an object containing one must also retain its address. The optional
non-copyable member consequently makes `GladeObject` non-copyable.

`CollisionWorld` holds non-owning registrations. Remove a collider before
destroying it, replacing it through `setCollider()`, clearing it, or destroying
its owner. Query results also reference colliders and must not outlive them.
World destruction and `clear()` do not destroy colliders.

For application objects, `Context::add()` queues registration with the renderer
and collision systems. Its normal request processing registers an object's
optional collider, `remove()` unregisters it, and context clears/state changes/
shutdown clear the world. Finalize initial collider positions before processing
the add request. Movement requires that registration has been processed first.
Game states and objects may borrow Context to access its collision world; they
must not outlive Context. Standalone worlds remain usable for focused tests.

`add()` caches world bounds; duplicate registration is a no-op. Call `update()`
after changing an owner's or collider's position, before any queries. Removing
an absent collider is a no-op; updating an absent collider throws
`std::invalid_argument`. The current broadphase scans these cached bounds.
Queries, registration and transform updates are intended for the same thread.

Enabled state, sensor state and category/mask filtering are evaluated at query
time and do not require `update()`. Disabled owners and colliders are excluded.
Sensors are excluded unless `CollisionFilter::includeSensors` is set. Both the
query's mask against the target category and the target's mask against the
query category must match. The query's category and mask are supplied explicitly;
they are not inferred from `ignoredCollider`.

## Query results

- `queryBounds()` returns broadphase candidates; it does not perform narrowphase.
- `queryOverlaps()` returns contacts for touching or penetrating shapes.
- `castShape()` checks the complete translation and returns the earliest
  selected contacts, with a fraction in `[0, 1]`. A miss has `hit == false`,
  fraction `1`, and no contacts. Filters apply to all three operations.

Overlap results identify the encountered collider. A manifold's unit normal
points from that collider towards the query shape. Contact positions are on
the encountered shape's surface in physical X/Y, with Z zero. Separation is
negative for penetration and zero for touching. Rectangle face contacts have
one or two endpoints. For penetrating rectangles, one minimum-separation axis
is returned; equal depths choose X, and coincident centres choose the negative
direction deterministically. Containment uses face separation distance rather
than intersection width.

Casts report initial penetration at fraction zero with `startsOverlapping`
set. Touching at the start blocks only movement into the surface; stationary,
outward and tangential movement do not produce a blocking hit. An isolated
corner graze does not block. A contact reached exactly at the destination does
count if continuing along the path would enter the obstacle.

Simultaneous cast contacts retain separate normals, including two faces of a
single rectangle. Equal reported hit fractions are collected across colliders;
contact ordering follows registration order and is not a priority rule.
Calculations use floating-point coordinates, so callers should use a practical
world scale. Detection queries themselves do not apply a collision skin or
movement tolerance.

## Kinematic movement

`KinematicMover` holds a non-owning reference to a `CollisionWorld`. Its collider
must be registered, enabled, non-sensor and `KINEMATIC`, with an enabled owner.
Invalid collider preconditions or settings throw `std::invalid_argument`.
The mover refreshes its collider's bounds at entry and after position changes;
callers remain responsible for updating any other changed colliders first.

`move(collider, request)` consumes a displacement in world units, without an
input device, timer, velocity, gravity or movement-mode dependency. Only X/Y
are applied. Z is ignored and preserved on the owner. The local collider
transform, rotation and scale are preserved. Filtering comes from the collider's
category and mask, excludes itself, and excludes sensors.

Movement casts along the remaining displacement, advances with a small clearance
along blocking normals, removes inward displacement against encountered normals,
and casts again to slide. The representable candidate position is checked for
penetration before committing each step. Movement constraints support arbitrary
X/Y normals. In 2D the mover chooses the closest displacement satisfying all accumulated contact normals, considering
each constraint boundary and zero displacement. This avoids a later projection
reintroducing inward movement against an earlier, non-perpendicular surface.
This is boundary sliding, not elevation, grounded state or ramp support.
`KinematicMoverSettings::maxSlideAngleDegrees` limits sliding by the acute angle
between the original requested X/Y direction and each blocking surface's tangent.
The limit is inclusive, finite and in [0, 90] degrees; the default 90 preserves
unrestricted sliding. Above the limit the mover advances to safe contact and
stops the rest of the request with `BLOCKED`. At or below it the mover applies
its usual constraints and recasts. Both stopping and sliding can report `BLOCKED`.
The original direction remains the reference across successive contacts in one
move call. This is relative to input direction, not a fixed world-horizontal
incline or elevation rule. Zero requests and outward/tangential movement retain
the normal query behaviour. The angle comparison uses squared dot products;
only double-precision comparison roundoff is allowed at the inclusive boundary.

An advance too small to change the float owner position returns immediately
without overlap queries. For the pre-contact step this is treated as zero advance
and sliding may continue, rather than abandoning a valid tangential displacement.
Penetrating candidate advances remain rejected; a free movement that cannot
change the representable position still reports `ITERATION_LIMIT`.

Independent terrain pieces have no adjacency/internal-edge suppression; seamless
movement across arbitrary terrain decompositions is not guaranteed.

`MoveResult` records the original request, actual owner displacement and contact
snapshots encountered along the path. Contacts are not a final support query.
Statuses distinguish:

- `COMPLETED`: the supported X/Y movement completed without blocking.
- `BLOCKED`: contacts normally constrained some requested movement.
- `UNRESOLVED_OVERLAP`: penetration was detected. Initial penetration leaves
  the position unchanged; movement never invokes recovery automatically.
- `ITERATION_LIMIT`: the movement budget was exhausted or safe representable
  progress could not be made. Remaining movement is left unapplied.

`recoverOverlap(collider)` handles exceptional penetration explicitly. It selects
the deepest current overlap, corrects along its normal, and re-queries before
the next correction. Touching does not require recovery. Both the number of
corrections and the total distance travelled are bounded. It is best-effort:
trapped bodies can oscillate and remain unresolved rather than finding a global
escape route. Recovery does not cast its correction path or guarantee a valid
intermediate position; it is not a substitute for ordinary movement.

`RecoveryResult` distinguishes an already separated body, successful recovery
and unresolved penetration. The result reports the actual net correction and
the remaining penetrating contacts. Failure can leave a partial correction in
place; game rules decide whether to restore an earlier position or reject a spawn.

Settings require a finite positive contact tolerance, positive iteration limits,
and a finite nonnegative recovery distance. Defaults are `0.0001` world units,
eight movement iterations, eight recovery iterations and one world unit of
recovery travel. These values need validation against the eventual game's world
scale. The caller supplies timestep-consistent displacement; this class does not
introduce a simulation timestep or alter the application lifecycle.

## Example

```cpp
RectangleCollisionShape rectangle(0.5f, 0.25f);
GladeObject character(nullptr);
GladeObject wall(nullptr);
auto &character_collider = character.setCollider(rectangle, Collider::Type::KINEMATIC);
auto &wall_collider = wall.setCollider(rectangle);
wall.getTransform()->setPosition(0.75f, 0.0f, 0.0f);

CollisionWorld world;
world.add(character_collider);
world.add(wall_collider);

CollisionFilter filter;
filter.ignoredCollider = &character_collider;
filter.categoryBits = character_collider.getCategoryBits();
filter.maskBits = character_collider.getMaskBits();
Transform world_transform;
character_collider.getWorldTransform(world_transform);
const auto overlaps = world.queryOverlaps(rectangle, world_transform, filter);
const auto hit = world.castShape(rectangle, world_transform, {2.0f, 0.0f, 0.0f}, filter);

world.clear();
```

## Verification

Run the CPU-only test from the engine repository:

```sh
mkdir -p build
make collision-test > build/collision-test.log 2>&1
make kinematic-mover-test > build/kinematic-mover-test.log 2>&1
```

This compiles the real collision, `GladeObject`, logging and stream sources.
It needs no renderer, window server or vendor libraries. The test reports
failures through its exit status, including when assertions are disabled.
`tests/CMakeLists.txt` also exposes `glade_collision_test` and
`glade_kinematic_mover_test` when included by a host supplying the `glade` library target.

Coverage includes two objects with colliders, signed contacts and normals,
containment, local offsets and owner transforms, bounds updates, filters,
sensors, thin-obstacle casts, both X/Y axes, negative coordinates,
touching and corner cases, earliest and simultaneous hits, initial penetration,
registration, and invalid geometry inputs. An additional object exercises
simultaneous contacts against separate obstacles.

Mover tests cover free movement, transform preservation and bounds updates,
wall stopping, diagonal sliding, local offsets, high-speed thin obstacles in
both directions on both axes, simultaneous corners and successive obstacles,
movement iteration exhaustion, explicit recovery, touching, partial recovery,
multiple overlaps, trapping, filters and invalid inputs.

Polygon coverage additionally includes 625 rectangle/triangle placements and
1,089 rectangle/pentagon placements checked against independent geometric
oracles, manifold boundary points and separation vectors, winding/geometry
validation, containment, translated owner/local offsets, sensor/mask filtering,
rectangle/polygon queries in both directions, polygon/polygon queries, angled
sweeps, touching, endpoint contacts, corner grazing, simultaneous normals and
high-speed thin angled obstacles and a 32-vertex polygon. Mover coverage includes
45-degree and non-45-degree sliding,
repeated contact, moving away, angled recovery and an acute slope/ceiling pocket
with reversed registration order. Additional regressions cover limits just below,
exactly at and just above 45 degrees, original direction across successive faces,
diagonal input, defaults and invalid angles, nine constant and eight varied
rightward step sequences matching the prototype triangle, consistent 45-degree
stopping and the retained free-movement no-progress guard. These are CPU tests, not rendered terrain or
level-authoring integration. No vertical-body collision, event handling or input
controller is implemented by this module.
