#include <glade/util/ResourceManager.h>
#include <glade/util/GladeTexAtlasReader.h>
#include <glade/render/TextureAtlas.h>
#include <filesystem>
#include <glade/util/Path.h>
#include <glade/util/DesktopFileManager.h>
#include <glade/util/WavefrontObjReader.h>
#include <glade/render/ShaderProgram.h>
#include <glade/render/Texture.h>
#include <glade/render/meshes/Triangle.h>
#include <glade/render/meshes/Rectangle.h>
#include <glade/render/meshes/Cube.h>
#include <glade/render/meshes/Square.h>
#include <glade/render/meshes/Mesh.h>
#include <glade/ui/font/BitmapFont.h>
#include <glade/ui/font/FreetypeFont.h>
#include <glade/audio/Sound.h>
#include <glade/exception/GladeException.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <lodepng.h>

Glade::ResourceManager::ResourceManager(FileManager *file_manager) : fileManager(file_manager) {}

std::shared_ptr<Texture> Glade::ResourceManager::getTexture(const Path &filename) {
  TexturesI i = textures.find(filename);

  if (i != textures.end()) {
    return i->second;
  }

  std::shared_ptr<Texture> texture = loadTexture(filename);
  textures[filename] = texture;

  return texture;
}

std::shared_ptr<const Glade::TextureAtlas>
Glade::ResourceManager::getTextureAtlas(const Path &filename) {
  const auto existing = textureAtlases.find(filename);

  if (existing != textureAtlases.end()) return existing->second;

  try {
    std::vector<char> json;
    fileManager->getFileContents(filename, json);
    TextureAtlasMetadata metadata;
    GladeTexAtlasReader::read(json, metadata);

    // Image paths are relative to the metadata file, not the process directory.
    const std::filesystem::path image_path(Path(metadata.textureFilename).toString());

    if (image_path.is_absolute() || image_path.has_root_name())
      throw std::invalid_argument("texture must be a relative path");

    const auto resolved =
        (std::filesystem::path(filename.toString()).parent_path() / image_path).lexically_normal();
    auto texture = getTexture(Path(resolved.string()));
    auto atlas = std::make_shared<const TextureAtlas>(texture, metadata);
    textureAtlases.emplace(filename, atlas);

    return atlas;
  } catch (GladeException &error) {
    std::string message = "GladeTexAtlas " + filename.toString() + ": " + error.getMessage();
    throw GladeException(message);
  } catch (const std::exception &error) {
    std::string message = "GladeTexAtlas " + filename.toString() + ": " + error.what();
    throw GladeException(message);
  }
}

std::shared_ptr<Glade::Mesh> Glade::ResourceManager::getMesh(MeshType type) {
  BuiltinMeshesI i = builtInMeshes.find(type);

  if (i != builtInMeshes.end()) {
    return i->second;
  }

  std::shared_ptr<Mesh> mesh = loadMesh(type);
  builtInMeshes[type] = mesh;

  return mesh;
}

std::shared_ptr<Glade::Mesh> Glade::ResourceManager::getMesh(const Path &filename) {
  MeshesI i = meshes.find(filename);

  if (i != meshes.end()) {
    //return i->second;
    return std::shared_ptr<Mesh>(i->second->clone());
  }

  std::shared_ptr<Mesh> mesh = loadMeshAssimp(filename);
  meshes[filename] = mesh;

  return mesh;
}

std::shared_ptr<ShaderProgram>
Glade::ResourceManager::getShaderProgram(const Path &vertex_shader_filename,
                                         const Path &fragment_shader_filename) {
  Path full_path_to_vertex_shader = getShadersSubfolder() + vertex_shader_filename;
  Path full_path_to_fragment_shader = getShadersSubfolder() + fragment_shader_filename;
  Path key = full_path_to_vertex_shader + ";" + full_path_to_fragment_shader;
  ShaderProgramsI i = shaderPrograms.find(key);

  if (i != shaderPrograms.end()) {
    return i->second;
  }

  std::shared_ptr<ShaderProgram> shader_program =
      loadShaderProgram(full_path_to_vertex_shader, full_path_to_fragment_shader);
  shaderPrograms[key] = shader_program;

  return shader_program;
}

Path Glade::ResourceManager::getShadersSubfolder() const { return Path("shaders/gl"); }

std::shared_ptr<Sound> Glade::ResourceManager::getSound(const Path &filename) {
  SoundsI i = sounds.find(filename);

  if (i != sounds.end()) {
    return i->second;
  }

  std::shared_ptr<Sound> sound = loadSound(filename);
  sounds[filename] = sound;

  return sound;
}

std::shared_ptr<Font> Glade::ResourceManager::getBitmapFont(const Path &atlas_filename,
                                                            const Path &csv_filename,
                                                            float viewport_width,
                                                            float viewport_height) {
  Path key = atlas_filename + ";" + csv_filename;
  BitmapFontsI i = bitmapFonts.find(key);

  if (i != bitmapFonts.end()) {
    return i->second;
  }

  std::shared_ptr<Font> bitmap_font =
      loadBitmapFont(atlas_filename, csv_filename, viewport_width, viewport_height);
  bitmapFonts[key] = bitmap_font;

  return bitmap_font;
}

std::shared_ptr<Font> Glade::ResourceManager::getDynamicFont(const Path &font_filename,
                                                             float viewport_width,
                                                             float viewport_height) {
  DynamicFontsI i = dynamicFonts.find(font_filename);

  if (i != dynamicFonts.end()) {
    return i->second;
  }

  std::shared_ptr<Font> dynamic_font =
      loadDynamicFont(font_filename, viewport_width, viewport_height);
  dynamicFonts[font_filename] = dynamic_font;

  return dynamic_font;
}

std::shared_ptr<ShaderProgram>
Glade::ResourceManager::loadShaderProgram(const Path &vertex_shader_filename,
                                          const Path &fragment_shader_filename) {
  std::shared_ptr<ShaderProgram> shader_program(new ShaderProgram());

  fileManager->getFileContents(vertex_shader_filename, shader_program.get()->vertexShaderSource);
  fileManager->getFileContents(fragment_shader_filename,
                               shader_program.get()->fragmentShaderSource);

  return shader_program;
}

std::shared_ptr<Texture> Glade::ResourceManager::loadTexture(const Path &filename) {
  std::vector<unsigned char> png_data;
  fileManager->getFileContents(filename, png_data, true);

  std::vector<unsigned char> pixels;
  unsigned width, height;

  unsigned error = lodepng::decode(pixels, width, height, png_data);

  if (error) {
    std::string exception_text;
    exception_text += "LodePNG error: ";
    exception_text += ::lodepng_error_text(error);
    throw GladeException(exception_text.c_str());
  }

  return std::make_shared<Texture>(width, height, pixels);
}

std::shared_ptr<Glade::Mesh> Glade::ResourceManager::loadMesh(MeshType type) {
  switch (type) {
  case MESH_TRIANGLE:

    return std::shared_ptr<Mesh>(new Triangle());
  case MESH_RECTANGLE:

    return std::shared_ptr<Mesh>(new Rectangle());
  case MESH_CUBE:

    return std::shared_ptr<Mesh>(new Cube());
  case MESH_SQUARE:

    return std::shared_ptr<Mesh>(new Square());
  default:
    throw GladeException("Unsupported mesh type requested");
  }
}

std::shared_ptr<Glade::Mesh> Glade::ResourceManager::loadMesh(const Path &filename) {
  std::vector<char> obj_data;
  fileManager->getFileContents(filename, obj_data);
  Mesh *mesh = new Mesh();

  if (!WavefrontObjReader::read(obj_data, mesh)) {
    throw GladeException("Failed to read Wavefront OBJ file");
  }

  return std::shared_ptr<Mesh>(mesh);
}

std::shared_ptr<Glade::Mesh> Glade::ResourceManager::loadMeshAssimp(const Path &filename) {
  ::log("Loading mesh with assimp");
  Assimp::Importer importer;

  Path file_path = ((DesktopFileManager *)fileManager)->getAbsolutePath(filename);
  const aiScene *scene = importer.ReadFile(
      file_path.toString(), aiProcess_Triangulate /*| aiProcess_JoinIdenticalVertices*/);

  if (nullptr == scene) {
    ::log("Failed to read Wavefront OBJ file with assimp from file %s: %s",
          filename.toString().c_str(), importer.GetErrorString());

    return nullptr;
  }

  if (!scene->HasMeshes()) {
    ::log("No mesh found in file %s", filename.toString().c_str());
    return nullptr;
  }

  ::log("Found %d meshes, will take only the first one", scene->mNumMeshes);

  const aiMesh *mesh = scene->mMeshes[0];

  ::log("Found %d vertices in the mesh", mesh->mNumVertices);
  ::log("Found %d faces in the mesh", mesh->mNumFaces);

  std::shared_ptr<Mesh> glade_mesh = std::make_shared<Mesh>();

  for (int i = 0; i < mesh->mNumVertices; ++i) {
    //log("Vertex: %2.1f, %2.1f, %2.1f", mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
    //log("Normal: %2.1f, %2.1f, %2.1f", mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

    if (mesh->mTextureCoords[i]) {
      ::log("TexCoord: %2.1f, %2.1f, %2.1f", mesh->mTextureCoords[0][i].x,
            mesh->mTextureCoords[0][i].y);
    } else {
      ::log("No texture coords found in the vertex");
    }

    // position
    glade_mesh->vertices.push_back(mesh->mVertices[i].x);
    glade_mesh->vertices.push_back(mesh->mVertices[i].y);
    glade_mesh->vertices.push_back(mesh->mVertices[i].z);

    // normal
    glade_mesh->vertices.push_back(mesh->mNormals[i].x);
    glade_mesh->vertices.push_back(mesh->mNormals[i].y);
    glade_mesh->vertices.push_back(mesh->mNormals[i].z);

    // tex coord
    glade_mesh->vertices.push_back(mesh->mTextureCoords[0][i].x);
    glade_mesh->vertices.push_back(mesh->mTextureCoords[0][i].y);
    //gladeMesh->vertices.push_back(0.0f);
    //gladeMesh->vertices.push_back(0.0f);
  }

  for (int i = 0; i < mesh->mNumFaces; ++i) {
    aiFace &face = mesh->mFaces[i];

    if (face.mNumIndices != 3) {
      ::log("Warning: non-triangle face found in a mesh");
      //return nullptr;
    }

    //log("Face: %d %d %d", face.mIndices[0], face.mIndices[1], face.mIndices[2]);

    glade_mesh->indices.push_back(face.mIndices[0]);
    glade_mesh->indices.push_back(face.mIndices[1]);
    glade_mesh->indices.push_back(face.mIndices[2]);
  }

  return std::shared_ptr<Glade::Mesh>(glade_mesh);
}

std::shared_ptr<Font> Glade::ResourceManager::loadBitmapFont(const Path &atlas_filename,
                                                             const Path &csv_filename,
                                                             float viewport_width,
                                                             float viewport_height) {
  std::vector<char> raw_csv;
  fileManager->getFileContents(csv_filename, raw_csv);
  std::vector<std::vector<std::string>> parsed_csv;
  CSVReader::read(raw_csv, parsed_csv);

  int declared_atlas_width = ::atoi(parsed_csv[0][1].c_str());
  int declared_atlas_height = ::atoi(parsed_csv[1][1].c_str());
  int cell_width = ::atoi(parsed_csv[2][1].c_str());
  int cell_height = ::atoi(parsed_csv[3][1].c_str());

  std::shared_ptr<Texture> atlas = getTexture(atlas_filename);

  if (declared_atlas_width != atlas->textureWidth) {
    throw GladeException("CSV data doesn't match atlas data");
  }

  if (declared_atlas_height != atlas->textureHeight) {
    throw GladeException("CSV data doesn't match atlas data");
  }

  if (cell_width == 0 || cell_height == 0 || atlas == nullptr || cell_width > atlas->textureWidth ||
      cell_height > atlas->textureHeight) {
    throw GladeException("Invalid values for BitmapFont constructor");
  }

  std::shared_ptr<Font> result(new BitmapFont(atlas, parsed_csv, viewport_width, viewport_height));

  return result;
}

std::shared_ptr<Font> Glade::ResourceManager::loadDynamicFont(const Path &font_filename,
                                                              float viewport_width,
                                                              float viewport_height) {
  std::vector<unsigned char> *face_buffer = new std::vector<unsigned char>();
  fileManager->getFileContents(font_filename, *face_buffer, true);
  std::unique_ptr<std::vector<unsigned char>> face_buffer_ptr(face_buffer);

  std::shared_ptr<FreetypeFont> font(
      new FreetypeFont(face_buffer_ptr, viewport_width, viewport_height));

  return font;
}

std::shared_ptr<Sound> Glade::ResourceManager::loadSound(const Path &filename) {
  // todo implement
  return std::shared_ptr<Sound>(new Sound());
}
