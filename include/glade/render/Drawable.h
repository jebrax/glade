#pragma once

#include <string>
#include <map>
#include <memory>
#include <optional>
#include "SpriteAnimator.h"

#include "glade/math/Transform.h"
#include "meshes/Mesh.h"
#include "Texture.h"
#include "TextureTransform.h"
#include "ShaderProgram.h"

struct CubeMapTexture {
  std::shared_ptr<Texture> top;
  std::shared_ptr<Texture> bottom;
  std::shared_ptr<Texture> left;
  std::shared_ptr<Texture> right;
  std::shared_ptr<Texture> front;
  std::shared_ptr<Texture> back;
};

class Drawable {
public:
  typedef std::map<std::string, float> ShaderFloatUniforms;
  typedef std::map<std::string, bool> ShaderBoolUniforms;
  typedef std::map<std::string, int> ShaderIntUniforms;
  typedef std::map<std::string, Glade::Vector3f> ShaderVec3Uniforms;
  typedef std::map<std::string, Glade::Vector4f> ShaderVec4Uniforms;
  typedef std::map<std::string, std::shared_ptr<Texture>> Shader2DTextureUniforms;
  typedef std::map<std::string, CubeMapTexture> ShaderCubeMapTextureUniforms;

  typedef ShaderFloatUniforms::const_iterator ShaderFloatUniformsCI;
  typedef ShaderBoolUniforms::const_iterator ShaderBoolUniformsCI;
  typedef ShaderIntUniforms::const_iterator ShaderIntUniformsCI;
  typedef ShaderVec3Uniforms::const_iterator ShaderVec3UniformsCI;
  typedef ShaderVec4Uniforms::const_iterator ShaderVec4UniformsCI;
  typedef Shader2DTextureUniforms::const_iterator Shader2DTextureUniformsCI;
  typedef ShaderCubeMapTextureUniforms::const_iterator ShaderCubeMapTextureUniformsCI;

  bool preservePosition, preserveRotation, preserveScale;

protected:
  Transform transform;
  std::shared_ptr<Glade::Mesh> mesh;
  TextureTransform texture2DTransform;
  std::optional<Glade::SpriteAnimator> spriteAnimator;
  std::shared_ptr<ShaderProgram> shaderProgram;

private:
  ShaderFloatUniforms shaderFloatUniforms;
  ShaderBoolUniforms shaderBoolUniforms;
  ShaderIntUniforms shaderIntUniforms;
  ShaderVec3Uniforms shaderVec3Uniforms;
  ShaderVec4Uniforms shaderVec4Uniforms;
  Shader2DTextureUniforms shader2DTextureUniforms;
  ShaderCubeMapTextureUniforms shaderCubeMapTextureUniforms;

public:
  Drawable() : preservePosition(false), preserveRotation(false), preserveScale(false) {}

  Drawable(std::shared_ptr<Glade::Mesh> mesh_, std::shared_ptr<ShaderProgram> shader_program)
      : mesh(mesh_), shaderProgram(shader_program), preservePosition(false),
        preserveRotation(false), preserveScale(false) {}

  ShaderFloatUniformsCI floatUniformsBegin() { return shaderFloatUniforms.cbegin(); }
  ShaderFloatUniformsCI floatUniformsEnd() { return shaderFloatUniforms.cend(); }
  ShaderBoolUniformsCI boolUniformsBegin() { return shaderBoolUniforms.cbegin(); }
  ShaderBoolUniformsCI boolUniformsEnd() { return shaderBoolUniforms.cend(); }
  ShaderIntUniformsCI intUniformsBegin() { return shaderIntUniforms.cbegin(); }
  ShaderIntUniformsCI intUniformsEnd() { return shaderIntUniforms.cend(); }
  ShaderVec3UniformsCI vec3UniformsBegin() { return shaderVec3Uniforms.cbegin(); }
  ShaderVec3UniformsCI vec3UniformsEnd() { return shaderVec3Uniforms.cend(); }
  ShaderVec4UniformsCI vec4UniformsBegin() { return shaderVec4Uniforms.cbegin(); }
  ShaderVec4UniformsCI vec4UniformsEnd() { return shaderVec4Uniforms.cend(); }
  Shader2DTextureUniformsCI texture2DUniformsBegin() { return shader2DTextureUniforms.cbegin(); }
  Shader2DTextureUniformsCI texture2DUniformsEnd() { return shader2DTextureUniforms.cend(); }

  ShaderCubeMapTextureUniformsCI textureCubeMapUniformsBegin() {
    return shaderCubeMapTextureUniforms.cbegin();
  }

  ShaderCubeMapTextureUniformsCI textureCubeMapUniformsEnd() {
    return shaderCubeMapTextureUniforms.cend();
  }

  void setUniform(const std::string &name, float val) { shaderFloatUniforms[name] = val; }
  void setUniform(const std::string &name, bool val) { shaderBoolUniforms[name] = val; }
  void setUniform(const std::string &name, int val) { shaderIntUniforms[name] = val; }

  void setUniform(const std::string &name, const Glade::Vector3f &val) {
    shaderVec3Uniforms[name] = val;
  }

  void setUniform(const std::string &name, const Glade::Vector4f &val) {
    shaderVec4Uniforms[name] = val;
  }

  void setUniform(const std::string &name, const std::shared_ptr<Texture> texture) {
    shader2DTextureUniforms[name] = texture;
  }

  void setUniform(const std::string &name, const CubeMapTexture &texture) {
    shaderCubeMapTextureUniforms[name] = texture;
  }

  std::shared_ptr<ShaderProgram> getShaderProgram() { return shaderProgram; }
  TextureTransform *getTexture2DTransform(void) { return &texture2DTransform; }

  // Sprite drawables require the stock Rectangle and texcoord_frames shader.
  // Playback advances only when the owner calls animator.update(deltaMs).
  Glade::SpriteAnimator &setSpriteAtlas(std::shared_ptr<const Glade::TextureAtlas> atlas) {
    return spriteAnimator.emplace(std::move(atlas));
  }

  Glade::SpriteAnimator *getSpriteAnimator() { return spriteAnimator ? &*spriteAnimator : nullptr; }

  const Glade::SpriteAnimator *getSpriteAnimator() const {
    return spriteAnimator ? &*spriteAnimator : nullptr;
  }

  void clearSpriteAnimator() { spriteAnimator.reset(); }
  void setTransform(const Transform &transform_) { this->transform = transform_; }
  Transform *getTransform(void) { return &transform; }
  std::shared_ptr<Glade::Mesh> getMesh(void) { return mesh; }
};
