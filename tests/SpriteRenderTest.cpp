#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glade/render/GladeRenderer.h>
#include <glade/GladeObject.h>
#include <glade/system.h>
#include <glade/util/DesktopFileManager.h>
#include <glade/util/ResourceManager.h>
#include <glade/exception/GladeException.h>

#include <cstring>
#include <vector>
#include <stdexcept>
#include <cstdio>
#include <exception>
#include <memory>

// Legacy Glade font code expects these application-owned resource pointers.
FileManager* file_manager = nullptr;
Glade::ResourceManager* resource_manager = nullptr;

namespace {
GLFWwindow* window = nullptr;

void reportGlfwError(int code, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", code, description);
}

} // namespace

namespace Glade::System {
void getViewportSize(unsigned int* width, unsigned int* height) {
    int w = 0, h = 0;
    if (window) glfwGetFramebufferSize(window, &w, &h);
    *width = static_cast<unsigned int>(w);
    *height = static_cast<unsigned int>(h);
}
void toggleMouseCursor(bool enabled) {
    glfwSetInputMode(window, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}
void setMouseCursorPosition(double x, double y) { glfwSetCursorPos(window, x, y); }
void getMouseCursorPosition(double& x, double& y) { glfwGetCursorPos(window, &x, &y); }
} // namespace Glade::System


#include <array>
#include <cmath>

int main(int argc, char** argv) {
    if (argc != 2) return 2; // Asset directory containing shaders/gl.
    glfwSetErrorCallback(reportGlfwError);
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    window = glfwCreateWindow(128, 128, "Sprite renderer test", nullptr, nullptr);
    if (!window) return 1;
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return 1;
    while (glGetError() != GL_NO_ERROR) {} // GLEW core-context probing only.
    int result = 0;
    try {
        DesktopFileManager files{Path(argv[1])};
        Glade::ResourceManager resources(&files);
        resource_manager = &resources; file_manager = &files;
        Glade::Renderer renderer;
        renderer.onSurfaceCreated();
        renderer.setSceneProjectionMode(Glade::Renderer::ORTHO);
        renderer.setBackgroundColor(0, 0, 0);
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        renderer.onSurfaceChanged(width, height);
        Perception camera;
        renderer.setPerception(&camera);
        std::vector<unsigned char> source(8 * 8 * 4);
        for (int y = 0; y < 8; ++y) for (int x = 0; x < 8; ++x) {
            auto i = (y * 8 + x) * 4;
            source[i] = 20 + x * 25; source[i+1] = 20 + y * 25;
            source[i+2] = 70; source[i+3] = 255;
        }
        auto texture = std::make_shared<Texture>(8, 8, source);
        Glade::TextureAtlasMetadata metadata{"",
            std::vector<Glade::SpriteFrame>{{1, 2, 2, 3, 0.5, 2}, {4, 1, 4, 5, 3, 4}},
            Glade::TextureAtlas::Names{{"small", 0}, {"large", 1}},
            std::vector<Glade::SpriteAnimation>{{{{0, 10}, {1, 20}}, true}},
            Glade::TextureAtlas::Names{{"walk", 0}}};
        auto atlas = std::make_shared<const Glade::TextureAtlas>(texture, metadata);
        Drawable drawable(resources.getMesh(Glade::ResourceManager::MESH_RECTANGLE),
            resources.getShaderProgram("texcoord_frames.vertex.glsl", "textured.fragment.glsl"));
        drawable.getTransform()->setScale(0.1f, 0.1f, 1);
        auto& animator = drawable.setSpriteAtlas(atlas);
        GladeObject object(nullptr);
        object.getTransform()->setPosition(0, 0, -1);
        object.addDrawable(&drawable);
        renderer.add(&object);
        std::vector<unsigned char> rendered(width * height * 3);
        auto check = [&](const Glade::SpriteFrame* frame, bool flipX, bool flipY, bool selected) {
            drawable.getTexture2DTransform()->flipTextureHorizontally(flipX);
            drawable.getTexture2DTransform()->flipTextureVertically(flipY);
            // First-cycle engine diagnostics consume errors; the log is also
            // checked by the runner. Second draw retains errors for this check.
            renderer.onDrawFrame();
            renderer.onDrawFrame();
            glReadBuffer(GL_BACK); glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, rendered.data());
            if (glGetError() != GL_NO_ERROR) throw std::runtime_error("OpenGL error");
            unsigned mismatches = 0;
            for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
                const double lx = ((x + 0.5) / width * 2 - 1) / 0.1;
                const double ly = ((y + 0.5) / height * 2 - 1) / 0.1;
                std::array<unsigned char, 3> expected{0, 0, 0};
                if (frame && selected) {
                    const double fx = (flipX ? -lx : lx) + frame->pivotX;
                    const double fy = frame->pivotY - (flipY ? -ly : ly);
                    if (fx >= 0 && fx < frame->width && fy >= 0 && fy < frame->height) {
                        const auto i = ((frame->y + int(fy)) * 8 + frame->x + int(fx)) * 4;
                        for (int c = 0; c < 3; ++c) expected[c] = source[i+c];
                    }
                } else if (!frame && std::abs(lx) < 1 && std::abs(ly) < 1) {
                    const double u = (lx + 1) / 2, v = (ly + 1) / 2;
                    const int tx = std::min(7, int((flipX ? 1-u : u) * 8));
                    const int ty = std::min(7, int((flipY ? 1-v : v) * 8));
                    for (int c = 0; c < 3; ++c) expected[c] = source[(ty*8+tx)*4+c];
                }
                for (int c = 0; c < 3; ++c)
                    if (rendered[(y*width+x)*3+c] != expected[c]) ++mismatches;
            }
            std::printf("Renderer check: %u channel mismatches\n", mismatches);
            if (mismatches) throw std::runtime_error("Sprite pixel comparison failed");
        };
        check(&atlas->getFrame("small"), false, false, false); // No selection draws nothing.
        animator.play("walk");
        for (int frame = 0; frame < 2; ++frame) {
            for (int flip = 0; flip < 4; ++flip)
                check(&animator.getCurrentFrame(), flip & 1, flip & 2, true);
            animator.update(frame == 0 ? 10 : 20);
        }
        animator.setFrame("large");
        check(&animator.getCurrentFrame(), false, false, true);
        drawable.clearSpriteAnimator();
        drawable.setUniform("uTextureSampler0", texture);
        for (int flip = 0; flip < 4; ++flip) check(nullptr, flip & 1, flip & 2, true);
        // No sprite or texture, and a shader without UV uniforms.
        Drawable plain(resources.getMesh(Glade::ResourceManager::MESH_RECTANGLE),
            resources.getShaderProgram("texcoord_frames.vertex.glsl", "constant.fragment.glsl"));
        plain.setUniform("uColor", Glade::Vector4f(0, 1, 0, 1));
        GladeObject plainObject(nullptr);
        plainObject.getTransform()->setPosition(0, 0, -1);
        plainObject.addDrawable(&plain);
        renderer.remove(&object); renderer.add(&plainObject);
        renderer.onDrawFrame(); renderer.onDrawFrame();
        unsigned char pixel[3]; glReadPixels(width/2, height/2, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
        if (glGetError() != GL_NO_ERROR || pixel[0] || pixel[1] != 255 || pixel[2])
            throw std::runtime_error("Untextured drawable failed");
        renderer.remove(&plainObject);
        std::puts("Sprite renderer tests passed: atlas regions, size, pivot, both flips, playback, static selection, ordinary texture, untextured drawable.");
    } catch (GladeException& e) { std::fprintf(stderr, "%s\n", e.getMessage()); result = 1;
    } catch (const std::exception& e) { std::fprintf(stderr, "%s\n", e.what()); result = 1; }
    glfwDestroyWindow(window); glfwTerminate();
    return result;
}
