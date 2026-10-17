#include <glade/util/ResourceManager.h>
#include <glade/util/GladeTexAtlasReader.h>
#include <glade/render/TextureAtlas.h>
#include <glade/exception/GladeException.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <stdexcept>
#include <string>

FileManager* file_manager = nullptr;
Glade::ResourceManager* resource_manager = nullptr;
namespace Glade::System {
void getViewportSize(unsigned int* w, unsigned int* h) { *w = 128; *h = 128; }
void toggleMouseCursor(bool) {}
void setMouseCursorPosition(double, double) {}
void getMouseCursorPosition(double& x, double& y) { x = 0; y = 0; }
}

class MemoryFiles : public FileManager {
public:
    std::map<std::string, std::string> metadata;
    std::vector<unsigned char> image;
    mutable unsigned jsonReads = 0, imageReads = 0;
    void getFileContents(const Path&, std::ifstream&, bool) const override {
        throw std::logic_error("Stream not used by atlas loading");
    }
    void getFileContents(const Path& path, std::vector<char>& output, bool) const override {
        ++jsonReads;
        const auto it = metadata.find(path.toString());
        if (it == metadata.end()) throw GladeException("Missing test metadata");
        output.assign(it->second.begin(), it->second.end());
    }
    void getFileContents(const Path& path, std::vector<unsigned char>& output, bool) const override {
        ++imageReads;
        if (path.toString() != "nested/example_single.png")
            throw GladeException("Unexpected texture path");
        output = image;
    }
};

int main(int argc, char** argv) {
    if (argc != 2) return 2; // Any PNG >= 4 x 4, e.g. example_single.png.
    MemoryFiles files;
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) return 2;
    files.image.assign(std::istreambuf_iterator<char>(input), {});
    Glade::ResourceManager resources(&files);
    file_manager = &files; resource_manager = &resources;
    const std::string valid = R"({
      "format":"GladeTexAtlas", "version":1, "texture":"./example_single.png",
      "frames":{"a":{"x":0,"y":0,"width":2,"height":3,"pivot":{"x":-0.5,"y":4}}},
      "animations":{"walk":{"loop":true,"defaultFrameDuration":75.5,
        "frames":[{"frame":"a"},{"frame":"a","durationMs":120}]}}
    })";
    files.metadata["nested/atlas.json"] = valid;
    const auto atlas = resources.getTextureAtlas("nested/atlas.json");
    assert(atlas == resources.getTextureAtlas("nested/atlas.json"));
    assert(files.jsonReads == 1 && files.imageReads == 1);
    assert(atlas->getTexture() == resources.getTexture("nested/example_single.png"));
    assert(atlas->getFrame("a").pivotX == -0.5);
    assert(atlas->getAnimation("walk").frames[0].durationMs == 75.5);
    assert(atlas->getAnimation("walk").frames[1].durationMs == 120);

    unsigned rejected = 0;
    auto reject = [&](const std::string& json) {
        files.metadata["nested/bad.json"] = json;
        try {
            resources.getTextureAtlas("nested/bad.json");
            throw std::runtime_error("Invalid document was accepted");
        } catch (GladeException& e) {
            assert(std::string(e.getMessage()).find("nested/bad.json") != std::string::npos);
            ++rejected;
        }
    };
    auto replace = [&](const std::string& from, const std::string& to) {
        auto json = valid;
        const auto offset = json.find(from); assert(offset != std::string::npos);
        json.replace(offset, from.size(), to); reject(json);
    };
    reject(""); reject("{"); reject("[]"); reject(valid + " garbage");
    reject(valid + std::string(1, '\0') + "garbage");
    replace("\"version\":1", "\"version\":2");
    replace("\"version\":1", "\"version\":1.0");
    replace("GladeTexAtlas", "OtherFormat");
    replace("\"version\":1", "\"version\":1,\"version\":1");
    replace("\"version\":1", "\"version\":1,\"extra\":{\"x\":1,\"x\":2}");
    replace("\"width\":2", "\"width\":2.5");
    replace("\"width\":2", "\"width\":2147483648");
    replace("\"width\":2", "\"width\":0");
    replace("\"x\":0", "\"x\":-1");
    replace("\"width\":2", "\"width\":2147483647"); // Outside actual image.
    replace("\"height\":3,", "");
    replace("\"loop\":true", "\"loop\":1");
    replace("\"frame\":\"a\"", "\"frame\":\"unknown\"");
    replace("\"durationMs\":120", "\"durationMs\":null");
    replace("\"durationMs\":120", "\"durationMs\":0");
    replace("\"defaultFrameDuration\":75.5", "\"defaultFrameDuration\":-1");
    replace("\"defaultFrameDuration\":75.5", "\"defaultFrameDuration\":\"75\"");
    replace("\"defaultFrameDuration\":75.5,", "");
    replace("\"durationMs\":120", "\"durationMs\":1e999");
    replace("\"x\":-0.5", "\"x\":null");
    replace("./example_single.png", "/absolute.png");
    replace("\"frames\":[{\"frame\":\"a\"},{\"frame\":\"a\",\"durationMs\":120}]", "\"frames\":[]");
    replace("\"a\":{\"x\":0,\"y\":0,\"width\":2,\"height\":3,\"pivot\":{\"x\":-0.5,\"y\":4}}", "");
    auto utf8 = valid; utf8.replace(utf8.find("GladeTexAtlas"), 1, 1, char(0xff)); reject(utf8);
    // Failed reads are not cached; repairing the same filename permits a retry.
    files.metadata["nested/bad.json"] = valid;
    assert(resources.getTextureAtlas("nested/bad.json")->getTexture() == atlas->getTexture());
    assert(files.imageReads == 1);
    std::cout << "Atlas reader tests passed: " << rejected << " invalid documents rejected; "
                 "relative paths, duration fallback/overrides, frame reuse, resource cache and retry verified.\n";
}
