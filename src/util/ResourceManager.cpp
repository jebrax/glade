#include <glade/util/ResourceManager.h>
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

Glade::ResourceManager::ResourceManager(FileManager *file_manager):
  fileManager(file_manager)
{
}

// TODO frame width and frame height must be stored within the texture on the disk.
// For now we enforce frame dimensions when the texture is initially loaded from disk
std::shared_ptr<Texture> Glade::ResourceManager::getTexture(const Path &filename, int frameWidth, int frameHeight)
{
  TexturesI i = textures.find(filename);
  
  if (i != textures.end()) {
    return i->second;
  }
  
  std::shared_ptr<Texture> texture = loadTexture(filename, frameWidth, frameHeight);
  textures[filename] = texture;
  return texture;
}

std::shared_ptr<Glade::Mesh> Glade::ResourceManager::getMesh(MeshType type)
{
  BuiltinMeshesI i = builtInMeshes.find(type);
  
  if (i != builtInMeshes.end()) {
    return i->second;
  }
  
  std::shared_ptr<Mesh> mesh = loadMesh(type);
  builtInMeshes[type] = mesh;
  return mesh;
}

std::shared_ptr<Glade::Mesh> Glade::ResourceManager::getMesh(const Path &filename)
{
  MeshesI i = meshes.find(filename);

  if (i != meshes.end()) {
    //return i->second;
    return std::shared_ptr<Mesh>(i->second->clone());
  }

  std::shared_ptr<Mesh> mesh = loadMeshAssimp(filename);
  meshes[filename] = mesh;
  return mesh;
}

std::shared_ptr<ShaderProgram> Glade::ResourceManager::getShaderProgram(const Path &vertex_shader_filename, const Path &fragment_shader_filename)
{
  Path fullPathToVertexShader = getShadersSubfolder() + vertex_shader_filename;
  Path fullPathToFragmentShader = getShadersSubfolder() + fragment_shader_filename;
  Path key = fullPathToVertexShader + ";" + fullPathToFragmentShader;
  ShaderProgramsI i = shaderPrograms.find(key);
  
  if (i != shaderPrograms.end()) {
    return i->second;
  }
  
  std::shared_ptr<ShaderProgram> shaderProgram = loadShaderProgram(fullPathToVertexShader, fullPathToFragmentShader);
  shaderPrograms[key] = shaderProgram;
  return shaderProgram;
}

Path Glade::ResourceManager::getShadersSubfolder() const
{
  return Path("shaders/gl");
}

std::shared_ptr<Sound> Glade::ResourceManager::getSound(const Path &filename)
{
  SoundsI i = sounds.find(filename);
  
  if (i != sounds.end()) {
    return i->second;
  }
  
  std::shared_ptr<Sound> sound = loadSound(filename);
  sounds[filename] = sound;
  return sound;
}

std::shared_ptr<Font> Glade::ResourceManager::getBitmapFont(const Path &atlas_filename, const Path &csv_filename, float viewport_width, float viewport_height)
{
  Path key = atlas_filename + ";" + csv_filename;
  BitmapFontsI i = bitmapFonts.find(key);
  
  if (i != bitmapFonts.end()) {
    return i->second;
  }
  
  std::shared_ptr<Font> bitmapFont = loadBitmapFont(atlas_filename, csv_filename, viewport_width, viewport_height);
  bitmapFonts[key] = bitmapFont;
  return bitmapFont;
}

std::shared_ptr<Font> Glade::ResourceManager::getDynamicFont(const Path &font_filename, float viewport_width, float viewport_height)
{
  DynamicFontsI i = dynamicFonts.find(font_filename);
  
  if (i != dynamicFonts.end()) {
    return i->second;
  }
  
  std::shared_ptr<Font> dynamicFont = loadDynamicFont(font_filename, viewport_width, viewport_height);
  dynamicFonts[font_filename] = dynamicFont;
  return dynamicFont;
}

std::shared_ptr<ShaderProgram> Glade::ResourceManager::loadShaderProgram(const Path &vertex_shader_filename, const Path &fragment_shader_filename)
{
  std::shared_ptr<ShaderProgram> shaderProgram(new ShaderProgram());

  fileManager->getFileContents(vertex_shader_filename, shaderProgram.get()->vertexShaderSource);
  fileManager->getFileContents(fragment_shader_filename, shaderProgram.get()->fragmentShaderSource);

  return shaderProgram;
}

std::shared_ptr<Texture> Glade::ResourceManager::loadTexture(const Path &filename, int frameWidth, int frameHeight)
{
  if (frameWidth < 0 || frameHeight < 0) {
    throw GladeException("Provided frameWidth and frameHeight must not be less than zero");
  }
  
  std::vector<unsigned char> pngData;
  fileManager->getFileContents(filename, pngData, true);
  
  std::vector<unsigned char> pixels;
  unsigned width, height;
  
  unsigned error = lodepng::decode(pixels, width, height, pngData);

  if (error) {
    std::string exceptionText;
    exceptionText += "LodePNG error: ";
    exceptionText += lodepng_error_text(error);
    throw GladeException(exceptionText.c_str());
  }
  
  if (frameWidth == 0) {  // if the user doesn't know frame dimensions
    frameWidth = width;   // we assume the whole texture is one frame
  } 
  
  if (frameHeight == 0) {
    frameHeight = height;
  }
  
  int numberOfFrames = width / frameWidth;
  numberOfFrames = numberOfFrames < 1 ? 1 : numberOfFrames;
  
  int numberOfAnimations = height / frameHeight;
  numberOfAnimations = numberOfAnimations < 1 ? 1 : numberOfAnimations;
  
  return std::shared_ptr<Texture>(new Texture(
    width,
    height,
    numberOfAnimations,
    numberOfFrames,
    pixels
  ));
}

std::shared_ptr<Glade::Mesh> Glade::ResourceManager::loadMesh(MeshType type)
{
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

std::shared_ptr<Glade::Mesh> Glade::ResourceManager::loadMesh(const Path &filename)
{
  std::vector<char> objData;
  fileManager->getFileContents(filename, objData);
  Mesh *mesh = new Mesh();

  if (!WavefrontObjReader::read(objData, mesh)) {
    throw GladeException("Failed to read Wavefront OBJ file");
  }

  return std::shared_ptr<Mesh>(mesh);
}

std::shared_ptr<Glade::Mesh> Glade::ResourceManager::loadMeshAssimp(const Path &filename)
{
  log("Loading mesh with assimp");
  Assimp::Importer importer;

  Path filePath = ((DesktopFileManager*)fileManager)->getAbsolutePath(filename);
  const aiScene* scene = importer.ReadFile(filePath.toString(), aiProcess_Triangulate /*| aiProcess_JoinIdenticalVertices*/);

  if (nullptr == scene) {
    log("Failed to read Wavefront OBJ file with assimp from file %s: %s", filename.toString().c_str(), importer.GetErrorString());
    return nullptr;
  }

  if (!scene->HasMeshes()) {
    log("No mesh found in file %s", filename.toString().c_str());
    return nullptr;
  }

  log("Found %d meshes, will take only the first one", scene->mNumMeshes);

  const aiMesh *mesh = scene->mMeshes[0];

  log("Found %d vertices in the mesh", mesh->mNumVertices);
  log("Found %d faces in the mesh", mesh->mNumFaces);

  std::shared_ptr<Mesh> gladeMesh = std::make_shared<Mesh>();

  for (int i = 0; i < mesh->mNumVertices; ++i) {
    //log("Vertex: %2.1f, %2.1f, %2.1f", mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
    //log("Normal: %2.1f, %2.1f, %2.1f", mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

    // position
    gladeMesh->vertices.push_back(mesh->mVertices[i].x);   
    gladeMesh->vertices.push_back(mesh->mVertices[i].y);   
    gladeMesh->vertices.push_back(mesh->mVertices[i].z);   

    // normal
    gladeMesh->vertices.push_back(mesh->mNormals[i].x);   
    gladeMesh->vertices.push_back(mesh->mNormals[i].y);   
    gladeMesh->vertices.push_back(mesh->mNormals[i].z);   

    // tex coord
    gladeMesh->vertices.push_back(0.0f);   
    gladeMesh->vertices.push_back(0.0f);   
  }

  for (int i = 0; i < mesh->mNumFaces; ++i) {
    aiFace& face = mesh->mFaces[i];

    if (face.mNumIndices != 3) {
      log("Warning: non-triangle face found in a mesh");
      //return nullptr;
    }

    //log("Face: %d %d %d", face.mIndices[0], face.mIndices[1], face.mIndices[2]);

    gladeMesh->indices.push_back(face.mIndices[0]);
    gladeMesh->indices.push_back(face.mIndices[1]);
    gladeMesh->indices.push_back(face.mIndices[2]);
  }

  return std::shared_ptr<Glade::Mesh>(gladeMesh);
}

std::shared_ptr<Font> Glade::ResourceManager::loadBitmapFont(const Path &atlas_filename, const Path &csv_filename, float viewport_width, float viewport_height)
{
  std::vector<char> rawCsv;
  fileManager->getFileContents(csv_filename, rawCsv);
  std::vector<std::vector<std::string> > parsedCsv;
  CSVReader::read(rawCsv, parsedCsv);
  
  int declaredAtlasWidth  = ::atoi(parsedCsv[0][1].c_str());
  int declaredAtlasHeight = ::atoi(parsedCsv[1][1].c_str());
  int cellWidth           = ::atoi(parsedCsv[2][1].c_str());
  int cellHeight          = ::atoi(parsedCsv[3][1].c_str());
  
  std::shared_ptr<Texture> atlas = getTexture(atlas_filename, cellWidth, cellHeight);
          
  if (declaredAtlasWidth != atlas->textureWidth) {
    throw GladeException("CSV data doesn't match atlas data");
  }
  
  if (declaredAtlasHeight != atlas->textureHeight) {
    throw GladeException("CSV data doesn't match atlas data");
  }
  
  if (cellWidth == 0 || cellHeight == 0 || atlas == nullptr || cellWidth > atlas->textureWidth || cellHeight > atlas->textureHeight) {
    throw GladeException("Invalid values for BitmapFont constructor");
  }
  
  std::shared_ptr<Font> result(new BitmapFont(atlas, parsedCsv, viewport_width, viewport_height));
  
  return result;
}

std::shared_ptr<Font> Glade::ResourceManager::loadDynamicFont(const Path &font_filename, float viewport_width, float viewport_height)
{
  std::vector<unsigned char> *faceBuffer = new std::vector<unsigned char>();
  fileManager->getFileContents(font_filename, *faceBuffer, true);
  std::unique_ptr<std::vector<unsigned char> > faceBufferPtr(faceBuffer);
  
  std::shared_ptr<FreetypeFont> font(new FreetypeFont(
    faceBufferPtr,
    viewport_width,
    viewport_height
  ));
  
  return font;
}

std::shared_ptr<Sound> Glade::ResourceManager::loadSound(const Path &filename)
{
  // todo implement
  return std::shared_ptr<Sound>(new Sound());
}
