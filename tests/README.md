# Sprite atlas CPU tests

From the Glade repository root:

```sh
clang++ -std=c++20 -Wall -Wextra -Werror \
  -fsanitize=address,undefined -Iinclude tests/SpriteAtlasTest.cpp \
  -o /tmp/glade-atlas-test
/tmp/glade-atlas-test
```

The test supplies a no-op logging function and requires no OpenGL context.
Texture's existing disabled pixel cleanup is unchanged by this work.

The atlas constructor accepts a TextureAtlasMetadata reference containing
already resolved durations and frame indices. It validates before consuming the
metadata containers via moves; validation failure preserves the metadata.
JSON parsing, including defaultFrameDuration fallback and duplicate-key
validation, is handled by GladeTexAtlasReader and tested separately. TextureAtlas exposes only
const definitions. Keep it shared through `shared_ptr<const TextureAtlas>`;
SpriteAnimator retains that shared ownership for the lifetime of playback.

SpriteAnimator starts without an animation selected. `play(name)` starts or
restarts at entry zero, `pause()` preserves elapsed frame time, and `resume()`
continues a paused animation. A completed non-looping animation remains on its
last frame; call `play()` to restart it. Call `update(deltaMs)` once per simulation
update, not from rendering or frame getters. Negative or non-finite deltas are
rejected. Missing names/indices throw std::out_of_range; invalid atlas definitions
throw std::invalid_argument; reading a frame before selection throws
std::logic_error.

## Drawable and renderer integration

A Drawable owns optional playback, created with `setSpriteAtlas(atlas)` and
accessed with `getSpriteAnimator()`. Use `play(name)` for animation or
`setFrame(name)` for a static sprite. `clearSpriteAnimator()` restores ordinary
rendering. An animator with no selection renders nothing.

Sprites require the stock Rectangle mesh and texcoord_frames.vertex.glsl with
textured.fragment.glsl (or a compatible shader exposing the same uniforms).
The renderer obtains the image from the atlas; do not also bind it with
setUniform. Each pixel is one local geometry unit; use Drawable's existing
scale for world size. Frame size and pivot are applied before that transform.
TextureTransform's scale signs control sprite flips around the pivot. Its UV
scale magnitudes and offsets are ignored for sprites. For ordinary textures,
TextureTransform controls UV scale and offsets; flipping uses a signed scale
and a unit offset to mirror the full texture.

Texture now holds only image/GPU data; getTexture(filename) takes no grid
arguments. BitmapFont converts its existing glyph grid to explicit UVs.
JSON atlases are loaded through ResourceManager::getTextureAtlas(filename).

## Rendering test

SpriteRenderTest.cpp creates a hidden GLFW OpenGL 4.1 context and exercises the
real Renderer. Link it against Glade and its normal dependencies plus GLFW;
pass the engine asset directory (containing shaders/gl) as its sole argument.
It compares framebuffer pixels for differently sized, arbitrarily placed atlas
frames, fractional pivots, all flip combinations, animation transitions, static
selection, ordinary textures, and an untextured drawable. It needs window-server
access on macOS. Inspect output for GL errors as well as the exit status,
because existing renderer diagnostics can consume errors.

A host CMake build that already defines targets `glade` and `glfw` can add this
directory with `add_subdirectory(path/to/glade/tests glade-tests)`, then build
`glade_sprite_cpu_test` and `glade_sprite_render_test`. The rendering target
copies stock shaders beside its executable into `sprite-test-assets`.


## Atlas reader test

Build `glade_atlas_reader_test` through the same host CMake setup, then run it
with a PNG path as its sole argument (the prototype's example_single.png is
suitable). It uses an in-memory FileManager for metadata and the supplied PNG
for texture decoding, without an OpenGL context. Tests cover default and
explicit durations, shared frames, relative image paths, resource cache reuse,
retry after failure, and invalid format/version, types, ranges, references,
duplicate keys, syntax, encoding, and durations. The host must add the shared
vendor RapidJSON include directory privately to Glade's compiler includes.

## Standalone metadata parser test

`glade_atlas_metadata_test` links only GladeTexAtlasReader.cpp and the standard
library, without ResourceManager, FileManager, Texture, image decoding, or GL.
The host supplies RAPIDJSON_INCLUDE_DIR to build it. It can also be built directly:

```sh
clang++ -std=c++20 -Iinclude -I../../vendor/rapidjson/include \
  tests/GladeTexAtlasMetadataTest.cpp src/util/GladeTexAtlasReader.cpp \
  -o /tmp/glade-atlas-metadata-test
/tmp/glade-atlas-metadata-test
```

`GladeTexAtlasReader::read(json, metadata)` replaces the output reference only
on success. ResourceManager adds filename context to errors, resolves the
relative texture path, and loads the image before constructing the atlas.
The CPU atlas tests also check container storage is transferred without copies,
that the atlas survives metadata destruction, and that constructor validation
failure leaves metadata unchanged.
