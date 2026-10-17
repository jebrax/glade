#include <glade/util/GladeTexAtlasReader.h>
#include <glade/render/TextureAtlasMetadata.h>
#include <glade/exception/GladeException.h>

// Assimp may export a different RapidJSON version from its static library.
// Keep this private parser's template symbols distinct from those definitions.
#define RAPIDJSON_NAMESPACE GladeRapidJSON
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <algorithm>
#include <cmath>
#include <set>
#include <stdexcept>
#include <string>

namespace {
namespace json = GladeRapidJSON;
using json::Value;

[[noreturn]] void fail(const std::string& message) { throw std::invalid_argument(message); }

const Value& member(const Value& object, const char* key, const std::string& context) {
    if (!object.IsObject()) fail(context + " must be an object");
    const auto it = object.FindMember(key);
    if (it == object.MemberEnd()) fail(context + ": missing " + key);
    return it->value;
}

std::string string(const Value& value, const std::string& context) {
    if (!value.IsString()) fail(context + " must be a string");
    std::string result(value.GetString(), value.GetStringLength());
    if (result.empty() || result.find('\0') != std::string::npos)
        fail(context + " must be non-empty and contain no NUL characters");
    return result;
}

int integer(const Value& value, const std::string& context) {
    if (!value.IsInt()) fail(context + " must be an integer within signed 32-bit range");
    return value.GetInt();
}

double number(const Value& value, const std::string& context, bool positive = false) {
    if (!value.IsNumber()) fail(context + " must be a number");
    const double result = value.GetDouble();
    if (!std::isfinite(result) || (positive && result <= 0))
        fail(context + " must be finite" + (positive ? " and positive" : ""));
    return result;
}

void rejectDuplicateKeys(const Value& root) {
    // RapidJSON's DOM retains duplicate members. Inspect every object,
    // including unrecognised extension fields, before resolving any members.
    std::vector<const Value*> pending{&root};
    while (!pending.empty()) {
        const Value& value = *pending.back(); pending.pop_back();
        if (value.IsObject()) {
            std::set<std::string> names;
            for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it) {
                const auto name = string(it->name, "Object key");
                if (!names.insert(name).second) fail("Duplicate object key: " + name);
                pending.push_back(&it->value);
            }
        } else if (value.IsArray()) {
            for (const auto& entry : value.GetArray()) pending.push_back(&entry);
        }
    }
}
} // namespace

void Glade::GladeTexAtlasReader::read(
    const std::vector<char>& json, TextureAtlasMetadata& metadata) {
    try {
        if (json.empty()) fail("Empty JSON document");
        if (std::find(json.begin(), json.end(), '\0') != json.end())
            fail("JSON document contains a NUL byte");
        json::Document root;
        root.Parse<json::kParseValidateEncodingFlag | json::kParseFullPrecisionFlag>(
            json.data(), json.size());
        if (root.HasParseError())
            fail(std::string(json::GetParseError_En(root.GetParseError())) +
                 " at byte " + std::to_string(root.GetErrorOffset()));
        rejectDuplicateKeys(root);
        if (string(member(root, "format", "Atlas"), "format") != "GladeTexAtlas")
            fail("Unsupported atlas format");
        if (integer(member(root, "version", "Atlas"), "version") != 1)
            fail("Unsupported GladeTexAtlas version");
        TextureAtlasMetadata parsed;
        parsed.textureFilename = string(member(root, "texture", "Atlas"), "texture");
        auto& frames = parsed.frames;
        auto& frameNames = parsed.frameNames;
        const auto& definitions = member(root, "frames", "Atlas");
        if (!definitions.IsObject() || definitions.ObjectEmpty()) fail("frames must be a non-empty object");
        for (auto it = definitions.MemberBegin(); it != definitions.MemberEnd(); ++it) {
            const auto name = string(it->name, "Frame name");
            const auto context = "Frame " + name;
            const auto& f = it->value;
            const auto& pivot = member(f, "pivot", context);
            SpriteFrame frame{
                integer(member(f, "x", context), context + ".x"),
                integer(member(f, "y", context), context + ".y"),
                integer(member(f, "width", context), context + ".width"),
                integer(member(f, "height", context), context + ".height"),
                number(member(pivot, "x", context + ".pivot"), context + ".pivot.x"),
                number(member(pivot, "y", context + ".pivot"), context + ".pivot.y")};
            if (frame.x < 0 || frame.y < 0 || frame.width <= 0 || frame.height <= 0)
                fail(context + ": invalid rectangle");
            frameNames.emplace(name, frames.size()); frames.push_back(frame);
        }

        auto& animations = parsed.animations;
        auto& animationNames = parsed.animationNames;
        const auto& definitionsA = member(root, "animations", "Atlas");
        if (!definitionsA.IsObject() || definitionsA.ObjectEmpty()) fail("animations must be a non-empty object");
        for (auto it = definitionsA.MemberBegin(); it != definitionsA.MemberEnd(); ++it) {
            const auto name = string(it->name, "Animation name");
            const auto context = "Animation " + name;
            const auto& a = it->value;
            const auto& loop = member(a, "loop", context);
            if (!loop.IsBool()) fail(context + ".loop must be Boolean");
            const double defaultDuration = number(member(a, "defaultFrameDuration", context),
                context + ".defaultFrameDuration", true);
            const auto& sequence = member(a, "frames", context);
            if (!sequence.IsArray() || sequence.Empty()) fail(context + ".frames must be a non-empty array");
            SpriteAnimation animation{{}, loop.GetBool()};
            for (const auto& entry : sequence.GetArray()) {
                const auto frameName = string(member(entry, "frame", context), context + ".frame");
                const auto frame = frameNames.find(frameName);
                if (frame == frameNames.end()) fail(context + ": unknown frame " + frameName);
                const auto duration = entry.FindMember("durationMs");
                animation.frames.push_back({frame->second, duration == entry.MemberEnd() ?
                    defaultDuration : number(duration->value, context + ".durationMs", true)});
            }
            animationNames.emplace(name, animations.size()); animations.push_back(std::move(animation));
        }
        parsed.validate();
        metadata = std::move(parsed);
    } catch (const std::exception& error) {
        std::string message = std::string("GladeTexAtlas: ") + error.what();
        throw GladeException(message);
    }
}
