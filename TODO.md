# Renderer TODO

These findings came from the beatemup prototype: a green rectangle rendered
through Glade with an orthographic camera. The rectangle is visible, but the
framebuffer smoke test reports OpenGL errors. No renderer fixes have been
applied yet.

## Shader attributes and mesh upload

- [ ] Resolve shader attribute locations before configuring a mesh's vertex
  attributes. In `src/render/GladeRenderer.cpp`,
  `Renderer::moveIntoVideoMemory(GladeObject&)` currently uploads the mesh before
  compiling its shader program. Attribute locations can therefore be
  uninitialised or left over from another program. Compiling/resolving the
  program first is the immediate correction; ensure reused programs also have
  their locations selected before configuring the mesh.
- [ ] Bind the mesh's VAO and vertex buffer before calling
  `glVertexAttribPointer`. The mesh upload path currently unbinds the buffers
  before setting attribute pointers. Each pointer must capture the intended
  bound vertex buffer. Bind the index buffer while the intended VAO is active
  as well, since that binding belongs to the VAO. Review the binding order in
  both upload and draw paths, including unbinding operations in a core context.
- [ ] Store attribute locations as signed integers in
  `include/glade/render/GladeRenderer.h`. `glGetAttribLocation` returns `-1` for
  inactive or absent attributes; the current `uint32_t` fields turn this into a
  large positive value, making checks such as `aNormal >= 0` ineffective.
- [ ] Configure, enable, and disable an attribute only when its location is
  valid. Apply this consistently to position, normal, and texture coordinates
  in upload and drawing. A solid-colour shader need not use normals or UVs.
- [ ] Ensure VAO configuration remains correct when a mesh is shared by
  programs with different attribute locations. Either establish a consistent
  attribute layout or configure/cache bindings for the relevant program.

### Verification

- Render a position-only solid-colour shader and Glade's existing `lit_shape`
  shader pair. Check both expected framebuffer colours and the absence of
  OpenGL errors during upload, drawing, and shutdown.
- Exercise multiple objects/programs and a shared mesh to catch stale attribute
  locations and VAO bindings.
- Rerun `beatemup --smoke-test`. Before these fixes, the embedded position-only
  shader produced green centre pixels with error `0x502`
  (`GL_INVALID_OPERATION`); the existing `lit_shape` pair produced green centre
  pixels with error `0x501` (`GL_INVALID_VALUE`). These observations do not
  establish which individual call caused each error; check errors at the
  relevant calls when implementing the fixes.

## Built-in rectangle mesh

- [ ] Correct `Rectangle::getVertexBufferSize()` in
  `include/glade/render/meshes/Rectangle.h`. It returns 48 floats, but the actual
  data contains 32 (four vertices with eight floats each), causing the renderer
  to read beyond the array during upload. Derive the count from the data where
  practical so the declaration cannot drift.
- [ ] Review the rectangle's triangle winding. Its current indices are clockwise
  when viewed from positive Z, while the renderer uses counter-clockwise front
  faces and back-face culling. Make the intended visible face consistent with
  that convention and verify it with the default camera orientation. The
  prototype currently uses its own correctly sized, counter-clockwise mesh.
