// Standalone parser test: link only src/util/GladeTexAtlasReader.cpp.
#include <glade/util/GladeTexAtlasReader.h>
#include <glade/render/TextureAtlasMetadata.h>
#include <glade/exception/GladeException.h>

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

int main() {
    const std::string json = R"({
      "format":"GladeTexAtlas", "version":1, "texture":"sprites/axel.png",
      "frames":{"a":{"x":4,"y":2,"width":2,"height":3,"pivot":{"x":-0.5,"y":4}}},
      "animations":{"walk":{"loop":true,"defaultFrameDuration":75.5,
        "frames":[{"frame":"a"},{"frame":"a","durationMs":120}]}}
    })";
    Glade::TextureAtlasMetadata metadata;
    Glade::GladeTexAtlasReader::read({json.begin(), json.end()}, metadata);
    assert(metadata.textureFilename == "sprites/axel.png");
    assert(metadata.frames[0].x == 4 && metadata.frames[0].pivotX == -0.5);
    assert(metadata.animations[0].frames[0].frameIndex == 0);
    assert(metadata.animations[0].frames[0].durationMs == 75.5);
    assert(metadata.animations[0].frames[1].durationMs == 120);
    const auto* previousStorage = metadata.frames.data();
    bool rejected = false;
    try {
        const std::string invalid = "{\"format\":\"GladeTexAtlas\",\"version\":1}";
        Glade::GladeTexAtlasReader::read({invalid.begin(), invalid.end()}, metadata);
    } catch (GladeException&) { rejected = true; }
    assert(rejected && metadata.frames.data() == previousStorage);
    assert(metadata.textureFilename == "sprites/axel.png");
    // A second read replaces definitions instead of appending to them.
    Glade::GladeTexAtlasReader::read({json.begin(), json.end()}, metadata);
    assert(metadata.frames.size() == 1 && metadata.animations.size() == 1);
    // No image is needed to parse very large, otherwise valid rectangles.
    auto large = json;
    large.replace(large.find("\"width\":2"), 9, "\"width\":2147483647");
    Glade::GladeTexAtlasReader::read({large.begin(), large.end()}, metadata);
    assert(metadata.frames[0].width == 2147483647);
    std::cout << "Metadata parser passed without ResourceManager, FileManager, Texture, image decoding, or GL linkage.\n";
}
