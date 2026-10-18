# Sprite atlases and animation

Glade separates texture images, atlas definitions, and per-instance playback:

- `Texture` holds image data and the GPU texture handle. It contains no
  frame-grid or animation metadata.
- `TextureAtlasMetadata` holds the texture filename, frame definitions,
  animation definitions, and name-to-index maps.
- `TextureAtlas` combines a shared `Texture` with immutable frame and animation
  definitions. Multiple sprite instances can share the same atlas.
- `SpriteAnimator` holds playback state for one Drawable. Animators sharing an
  atlas can select and play animations independently.

A `SpriteFrame` defines an arbitrary pixel rectangle and a pivot relative to
its top-left corner. Atlas coordinates increase rightward and downward.
`SpriteAnimation` contains ordered `AnimationFrame` entries, each referencing a
frame by index and specifying its duration. Frame definitions can be reused;
animations can have different lengths and individual frame durations. Looping
animations repeat; non-looping animations finish with their last frame visible.

For the JSON format, parsing, validation, and resource loading, see
[GladeTexAtlas.md](GladeTexAtlas.md). `ResourceManager::getTextureAtlas()`
loads and caches shared atlas resources.

## Playback

Each `Drawable` optionally owns a `SpriteAnimator`, attached through
`setSpriteAtlas()`. Game updates explicitly call `update(deltaMs)` on the
animator; rendering and frame getters never advance playback. Use `play()` for
a named animation or `setFrame()` for a static atlas frame. An attached animator
without a selected frame makes that Drawable invisible; it does not stop other
Drawables from rendering. `clearSpriteAnimator()` restores ordinary rendering.

## Rendering

Sprite rendering uses the stock Rectangle mesh and
`texcoord_frames.vertex.glsl` with `textured.fragment.glsl`, or compatible
shaders exposing the same uniforms. The frame rectangle determines UVs; its
dimensions and pivot determine local geometry. One image pixel is one local
unit, and the Drawable's existing scale controls world size. The pivot remains
at the local origin when frames change, so changing frame dimensions does not
move the object's anchor. Frame geometry adjustments happen before the
Drawable's normal transform and do not modify its position or the shared mesh.

`TextureTransform` handles ordinary UV scaling, offsets, and flipping. For atlas
sprites, only its flipping applies; scale magnitudes and offsets are ignored.
Flipping mirrors the sprite around its pivot without sampling neighbouring
atlas regions. The renderer obtains the sprite texture from its atlas rather
than requiring a duplicate texture assignment through `setUniform()`.
The former uniform-grid loading and animation APIs have been removed; bitmap
fonts now express glyph selection through explicit UV transforms.

Keep parsing, resource ownership and loading, playback, and rendering decoupled.
Future additions should preserve these boundaries: parsing should remain
testable without images or resource services, shared atlas definitions should
remain independent of instance playback, and rendering should consume the
selected frame without advancing animation state.
