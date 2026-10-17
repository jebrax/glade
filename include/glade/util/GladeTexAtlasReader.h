#pragma once

#include <memory>
#include <vector>

class Path;

namespace Glade {
class ResourceManager;
class TextureAtlas;

// Format-specific parsing stays behind ResourceManager's resource facade.
class GladeTexAtlasReader {
public:
    // filename supplies the base directory for the texture path and diagnostics.
    // Throws GladeException on invalid metadata. Durations and frame references
    // are fully resolved before constructing the immutable atlas.
    static std::shared_ptr<const TextureAtlas> read(const std::vector<char>& json,
        const Path& filename, ResourceManager& resources);
};
} // namespace Glade
