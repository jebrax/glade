#pragma once

#include <vector>

namespace Glade {

struct TextureAtlasMetadata;

class GladeTexAtlasReader {
public:
  // Parses image-independent metadata. No files or resources are loaded.
  // Replaces metadata on success; leaves it unchanged on failure.
  // Throws GladeException on invalid JSON or definitions.
  static void read(const std::vector<char> &json, TextureAtlasMetadata &metadata);
};

} // namespace Glade
