#pragma once

#include <memory>
#include <map>

#include "FileManager.h"
#include "Path.h"

class Path;
class ShaderProgram;
class Texture;
class Sound;
class Font;

namespace Glade {

class Mesh;
class TextureAtlas;

class ResourceManager {
public:
  enum MeshType {
    MESH_TRIANGLE,
    MESH_RECTANGLE,
    MESH_CUBE,
    MESH_SQUARE,
  };

protected:
  FileManager *fileManager;

private:
  typedef std::map<Path, std::shared_ptr<Texture>> Textures;
  typedef Textures::iterator TexturesI;

  typedef std::map<MeshType, std::shared_ptr<Mesh>> BuiltinMeshes;
  typedef BuiltinMeshes::iterator BuiltinMeshesI;
  typedef std::map<Path, std::shared_ptr<Mesh>> Meshes;
  typedef Meshes::iterator MeshesI;

  typedef std::map<Path, std::shared_ptr<ShaderProgram>> ShaderPrograms;
  typedef ShaderPrograms::iterator ShaderProgramsI;

  typedef std::map<Path, std::shared_ptr<Sound>> Sounds;
  typedef Sounds::iterator SoundsI;

  typedef std::map<Path, std::shared_ptr<Font>> BitmapFonts;
  typedef BitmapFonts::iterator BitmapFontsI;

  typedef std::map<Path, std::shared_ptr<Font>> DynamicFonts;
  typedef DynamicFonts::iterator DynamicFontsI;

  std::map<Path, std::shared_ptr<const TextureAtlas>> textureAtlases;
  Textures textures;
  BuiltinMeshes builtInMeshes;
  Meshes meshes;
  ShaderPrograms shaderPrograms;
  Sounds sounds;
  BitmapFonts bitmapFonts;
  DynamicFonts dynamicFonts;

public:
  ResourceManager(FileManager *file_manager);
  std::shared_ptr<Texture> getTexture(const Path &filename);
  std::shared_ptr<const TextureAtlas> getTextureAtlas(const Path &filename);
  std::shared_ptr<Mesh> getMesh(MeshType type);
  std::shared_ptr<Mesh> getMesh(const Path &filename);

  std::shared_ptr<ShaderProgram> getShaderProgram(const Path &vertex_shader_filename,
                                                  const Path &fragment_shader_filename);

  std::shared_ptr<Sound> getSound(const Path &filename);

  std::shared_ptr<Font> getBitmapFont(const Path &atlas_filename, const Path &csv_filename,
                                      float viewport_width, float viewport_height);

  std::shared_ptr<Font> getDynamicFont(const Path &font_filename, float viewport_width,
                                       float viewport_height);

  Path getShadersSubfolder() const;

private:
  std::shared_ptr<ShaderProgram> loadShaderProgram(const Path &vertex_shader_filename,
                                                   const Path &fragment_shader_filename);

  std::shared_ptr<Texture> loadTexture(const Path &filename);
  std::shared_ptr<Mesh> loadMesh(MeshType type);
  std::shared_ptr<Mesh> loadMesh(const Path &filename);
  std::shared_ptr<Mesh> loadMeshAssimp(const Path &filename);
  std::shared_ptr<Sound> loadSound(const Path &filename);

  std::shared_ptr<Font> loadBitmapFont(const Path &atlas_filename, const Path &csv_filename,
                                       float viewport_width, float viewport_height);

  std::shared_ptr<Font> loadDynamicFont(const Path &font_filename, float viewport_width,
                                        float viewport_height);
};

}
