# Renderer TODO

These findings came from the beatemup prototype: a green rectangle rendered
through Glade with an orthographic camera. The attribute and buffer-binding
fixes were committed in `0d1692d`. The existing `lit_shape` smoke test now passes
without temporary diagnostics. The built-in rectangle issues and additional
verification below remain outstanding.

## Shader attributes and mesh upload

- [x] Resolve shader attribute locations before configuring a mesh's vertex
  attributes. `Renderer::moveIntoVideoMemory(GladeObject&)` in
  `src/render/GladeRenderer.cpp` now compiles the shader and refreshes its
  locations before uploading the mesh, including for reused programs.
- [x] Bind the mesh's VAO and vertex buffer before calling
  `glVertexAttribPointer`. Upload and drawing now bind the intended VAO before
  its buffers. Attribute pointers capture the bound vertex buffer, and the
  index buffer is bound while that VAO is active.
- [x] Store attribute locations as signed integers in
  `include/glade/render/GladeRenderer.h`, initialised to `-1`. The negative
  result for an inactive attribute is no longer converted to an unsigned index.
- [x] Configure, enable, and disable an attribute only when its location is
  valid. Position, normal, and texture-coordinate calls are guarded in both
  upload and drawing.
- [ ] Verify VAO configuration when a mesh is shared by programs with different
  attribute locations. Drawing currently refreshes the bindings for each
  program, but this scenario has not been exercised. Determine whether any
  further layout or caching changes are needed based on that test.

### Verification

- [x] Run `beatemup --smoke-test` with Glade's existing `lit_shape` shader pair.
  Expected green rectangle and black background verified; state shutdown
  completed. Per-call diagnostics captured zero errors after the fixes. After
  removing diagnostics and updating the submodule, the smoke test passed again.
- [ ] Rerun a position-only solid-colour shader after the fixes. Verify expected
  framebuffer colours and no OpenGL errors during upload, drawing, or shutdown.
- [ ] Exercise multiple objects/programs and a shared mesh to catch stale
  attribute locations and VAO bindings.

Before the fixes, the embedded position-only shader produced green centre
pixels with error `0x502` (`GL_INVALID_OPERATION`); the existing `lit_shape` pair
produced green centre pixels with error `0x501` (`GL_INVALID_VALUE`). Per-call
instrumentation of the latter isolated six invalid-index errors during upload
from unresolved attribute locations, and three errors per draw from using the
inactive UV attribute (`-1` converted to `4294967295`). Upload also configured
attribute pointers with vertex buffer zero bound. These findings motivated the
completed fixes above; the earlier `0x502` was not separately isolated.

## Built-in rectangle mesh

- [x] Derive the vertex and index buffer sizes from their C arrays using
  `std::size` for Rectangle, Cube, Square, and Triangle. This corrects Rectangle's
  reported vertex data from 48 to 32 floats and Triangle's from 36 to 24 floats.
  The shared inline static C arrays keep getters and counts tied to the same
  storage; geometry and triangle winding are unchanged.
- [x] Reverse Rectangle's triangle indices so both faces are counter-clockwise
  when viewed from positive Z, matching the renderer's front-face convention.
  Cross-product checks verify positive-Z geometric normals. Rendering the
  built-in mesh with the default camera remains to be verified; the prototype
  still uses its own mesh.
- [x] Set Square's four vertex normals to `(0, 0, 1)`, matching its positive-Z
  face winding and planar geometry.
