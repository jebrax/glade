# GladeTexAtlas version 1

GladeTexAtlas describes a sprite atlas in a separate JSON file. It contains
named frame definitions and named animations that reference those frames.
This document specifies the format implemented by GladeTexAtlasReader.

## Example

```json
{
  "format": "GladeTexAtlas",
  "version": 1,
  "texture": "axel.png",
  "frames": {
    "idle_0": {
      "x": 10,
      "y": 5,
      "width": 20,
      "height": 33,
      "pivot": { "x": 10, "y": 31 }
    },
    "punch_0": {
      "x": 40,
      "y": 5,
      "width": 45,
      "height": 33,
      "pivot": { "x": 12, "y": 31 }
    }
  },
  "animations": {
    "idle": {
      "loop": true,
      "defaultFrameDuration": 150,
      "frames": [
        { "frame": "idle_0" }
      ]
    },
    "punch": {
      "loop": false,
      "defaultFrameDuration": 100,
      "frames": [
        { "frame": "idle_0", "durationMs": 80 },
        { "frame": "punch_0" }
      ]
    }
  }
}
```

## Fields and conventions

All fields shown above are required except an animation entry's `durationMs`.

- `format` must be the string `"GladeTexAtlas"`; `version` must be the integer
  `1`. Loaders must reject unsupported format names or versions.
- `texture` identifies one image, using a path relative to the JSON file.
- `frames` is an object keyed by unique, non-empty frame names. Each definition
  supplies integer pixel coordinates `x`, `y`, `width`, and `height`. The atlas
  origin is its top-left corner; x increases rightward and y downward.
  Rectangles include their top-left edge and exclude their bottom-right edge.
- `pivot` supplies numeric pixel offsets `x` and `y` from the frame rectangle's
  top-left corner, using the same axis directions. It is the point aligned with
  the sprite object's position and the anchor for scaling, rotation, and
  flipping. Pivots may be fractional or lie outside the frame rectangle.
- `animations` is an object keyed by unique, non-empty animation names. Each
  animation has a Boolean `loop`, a `defaultFrameDuration` in milliseconds,
  and a non-empty ordered `frames` array.
- Each animation entry's `frame` references a name in the top-level `frames`
  object. Frames can be reused across animations or repeated within an animation.
- An entry's optional `durationMs` overrides `defaultFrameDuration` for that
  occurrence only. If absent, use the animation's `defaultFrameDuration`.
  Both duration properties must be finite, positive numbers; fractional
  milliseconds are allowed. An explicit `null` is invalid, not a fallback.
- Playback follows array order. A looping animation returns to its first entry
  after the last entry's duration; a non-looping animation stops on its last
  entry after that entry's duration and keeps it visible.

## Validation and scope

Reject duplicate JSON object keys, missing required fields, incorrect types,
non-finite numbers, unknown frame references, and empty frame or animation maps.
Frame origins must be non-negative, dimensions must be positive, and every
rectangle must fit entirely inside the referenced image.

Version 1 supports arbitrary frame sizes and positions, different animation
lengths, and per-entry timing. It does not support rotated atlas regions,
multiple images per atlas, collision shapes, or hitboxes.

## Loading in Glade

`ResourceManager::getTextureAtlas(filename)` returns a cached
`shared_ptr<const TextureAtlas>`. It reads through FileManager and delegates
parsing to `glade/util/GladeTexAtlasReader`. The reader fills a caller-supplied
`TextureAtlasMetadata&` with the texture filename, frames, animations, and name
maps; it has no resource-loading dependencies. Frame references and duration
defaults are resolved during parsing. ResourceManager resolves the texture path
relative to the metadata file, loads the image through its existing texture
cache, and constructs TextureAtlas with the texture and metadata reference.
TextureAtlas validates the metadata and image bounds before moving the metadata
containers into its own members without deep copies. The temporary metadata
is then discarded. Invalid metadata raises GladeException with the
metadata filename and a validation message (syntax errors include byte offsets).
Failed atlas loads are not cached.

The parser uses the official Tencent RapidJSON checkout at revision
`24b5e7a8b27f42fa16b96fc70aade9106cf7102f`, installed in the shared vendor
folder as `rapidjson` (including upstream licence). The Makefile adds
`${VENDOR}/rapidjson/include` only for GladeTexAtlasReader.cpp, matching its
existing per-source dependency rules. Supply VENDOR as usual, for example
`make VENDOR=../../vendor`. The parser uses RapidJSON's supported custom namespace
configuration privately in its .cpp file to avoid template symbol collisions
with the different RapidJSON version embedded in the existing Assimp library.
No RapidJSON types appear in Glade's public interface.
