#include <cstring>
#include <math.h>
#include <assert.h>
#include <algorithm>

#include <GL/glew.h>

#include "glade/debug/log.h"
#include "glade/math/util.h"
#include "glade/math/Matrix.h"
#include "glade/render/GladeRenderer.h"
#include "glade/render/Drawable.h"
#include "glade/render/meshes/Rectangle.h"
#include "glade/render/ShaderProgram.h"
#include "glade/render/Perception.h"
#include "glade/math/Vector.h"

const GLuint Glade::Renderer::POS_SIZE_FLOATS = 3;
const GLuint Glade::Renderer::NORMAL_SIZE_FLOATS = 3;
const GLuint Glade::Renderer::TEXCOORD_SIZE_FLOATS = 2;
const GLuint Glade::Renderer::POS_OFFSET_FLOATS = 0;
const GLuint Glade::Renderer::NORMAL_OFFSET_FLOATS = POS_SIZE_FLOATS;
const GLuint Glade::Renderer::TEXCOORD_OFFSET_FLOATS = POS_SIZE_FLOATS + NORMAL_SIZE_FLOATS;
const GLuint Glade::Renderer::VERTEX_STRIDE_FLOATS =
    POS_SIZE_FLOATS + NORMAL_SIZE_FLOATS + TEXCOORD_SIZE_FLOATS;
const GLvoid *Glade::Renderer::POS_OFFSET_BYTES =
    (const GLvoid *)(POS_OFFSET_FLOATS * sizeof(float));
const GLvoid *Glade::Renderer::NORMAL_OFFSET_BYTES =
    (const GLvoid *)(NORMAL_OFFSET_FLOATS * sizeof(float));
const GLvoid *Glade::Renderer::TEXCOORD_OFFSET_BYTES =
    (const GLvoid *)(TEXCOORD_OFFSET_FLOATS * sizeof(float));
const GLsizei Glade::Renderer::VERTEX_STRIDE_BYTES = VERTEX_STRIDE_FLOATS * sizeof(float);

static bool first_cycle;

Glade::Renderer::Renderer(void)
    : perception(NULL), initialized(false), sceneProjectionMode(PERSPECTIVE) {}

void Glade::Renderer::onSurfaceCreated() {
  ::glFrontFace(GL_CCW);
  ::glCullFace(GL_BACK);
  ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  ::glEnable(GL_BLEND);
  ::glEnable(GL_CULL_FACE);
  ::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  //::glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  //::glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
  moveAllObjectsIntoVideoMemory();
  ::log("Initialized renderer");
  initialized = true;
  checkGlError();
}

void Glade::Renderer::onSurfaceChanged(int width, int height) {
  viewportWidth = width;
  viewportHeight = height;
  aspectRatio = (float)viewportWidth / viewportHeight;
  ::glViewport(0, 0, viewportWidth, viewportHeight);

  switchProjectionMode(sceneProjectionMode, true);
  checkGlError();
}

void Glade::Renderer::add(GladeObject *p_scene_object) {
  if (this->initialized) {
    moveIntoVideoMemory(*p_scene_object);
  } else {
    ::log("Adding scene object, but renderer is not initialised");
  }

  sceneObjects.push_back(p_scene_object);
}

void Glade::Renderer::remove(GladeObject *scene_object) {
  std::vector<GladeObject *>::iterator oi =
      std::find(sceneObjects.begin(), sceneObjects.end(), scene_object);

  if (oi == sceneObjects.end()) {
    ::log("Warning: couldn't remove object, because it's not in the renderer");
    return;
  }

  GladeObject::Drawables *drawables = (*oi)->getDrawables();

  for (GladeObject::DrawablesI di = drawables->begin(); di != drawables->end(); di++) {
    removeFromVideoMemory(**di);
    checkGlError();
  }

  sceneObjects.erase(oi);
}

void Glade::Renderer::clear(void) {
  ::log("Clearing renderer");
  ::first_cycle = true;
  removeAllObjectsFromVideoMemory();

  sceneObjects.clear();

  checkGlError();

  ::log("Done clearing renderer");
}

void Glade::Renderer::onDrawFrame(void) {
  ::glClearColor(this->backgroundColor.x, this->backgroundColor.y, this->backgroundColor.z, 0);
  ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (getCamera()) {
    getCamera()->getMatrix(cameraTransform);
    getCamera()->getCameraMatrix(viewMatrix);
    switchProjectionMode(sceneProjectionMode);
    drawAll(sceneObjects.begin(), sceneObjects.end());
  } else if (::first_cycle) {
    ::log("Warning: no active camera, cannot render the scene");
  }

  ::glFlush();
  ::first_cycle = false;
}

void Glade::Renderer::setSceneProjectionMode(ProjectionMode projection_mode) {
  this->sceneProjectionMode = projection_mode;
}

int Glade::Renderer::getViewportWidth(void) { return viewportWidth; }

int Glade::Renderer::getViewportHeight(void) { return viewportHeight; }

void Glade::Renderer::setBackgroundColor(float r, float g, float b) {
  this->backgroundColor.set(r, g, b);
}

int Glade::Renderer::percentToPixels(float percent) {
  return (int)((viewportWidth < viewportHeight ? viewportWidth / 2 : viewportHeight / 2) * percent);
}

float Glade::Renderer::pixelsToPercent(float pixels) {
  return pixels / (viewportWidth < viewportHeight ? viewportWidth / 2 : viewportHeight / 2);
}

void Glade::Renderer::moveZeroToUpperLeftCorner(void) {
  getCamera()->setPosition(getHalfViewportWidthCoords(), getHalfViewportHeightCoords(), 0);
}

float Glade::Renderer::getHalfViewportWidthCoords(void) {
  return aspectRatio > 1 ? aspectRatio : 1;
}

float Glade::Renderer::getHalfViewportHeightCoords(void) {
  return aspectRatio > 1 ? 1 : aspectRatio;
}

float Glade::Renderer::getViewportWidthCoords(void) { return 2 * aspectRatio; }

float Glade::Renderer::getViewportHeightCoords(void) { return 2; }

Glade::Vector2f Glade::Renderer::getPointCoords(float screen_x, float screen_y) {
  Glade::Vector2f result;

  result.x = screen_x / viewportWidth * 2 * aspectRatio - aspectRatio;
  result.y = screen_y / viewportHeight * 2 - 1;

  return result;
}

Glade::Vector3f Glade::Renderer::unprojectPoint(float x, float y, float z) {
  float inv_view_matrix[16];
  float inv_proj_matrix[16];

  Glade::Vector4f point(x, y, z, 1.0);
  Glade::Vector4f result_point;

  Matrix::invertM(inv_view_matrix, 0, viewMatrix, 0);
  Matrix::invertM(inv_proj_matrix, 0, projectionMatrix, 0);
  Matrix::multiplyMv(result_point, inv_proj_matrix, point);
  result_point.w = 1.0;
  Matrix::multiplyMv(result_point, inv_view_matrix, result_point);

  return Glade::Vector3f(result_point.x, result_point.y, result_point.z);
}

void Glade::Renderer::drawAll(std::vector<GladeObject *>::iterator i,
                              std::vector<GladeObject *>::iterator end) {
  Transform final_world_transform_for_drawable;
  GladeObject::Drawables *drawables;

  while (i != end) {
    if ((*i)->isEnabled()) {
      drawables = (*i)->getDrawables();

      for (GladeObject::DrawablesI di = drawables->begin(); di != drawables->end(); di++) {
        if ((*i)->isViewEnabled(*di)) {
          (*i)->getTransform()->combineWith(*(*di)->getTransform(), (*di)->preservePosition,
                                            (*di)->preserveRotation, (*di)->preserveScale,
                                            final_world_transform_for_drawable);

          draw(di, final_world_transform_for_drawable);
        }
      }
    }

    i++;
  }
}

void Glade::Renderer::compileShaderProgram(ShaderProgram *program_) {
  if (program_ == nullptr || program_ == NULL) {
    ::log("Warning: tried to compile a shader program, but it's null");
  }

  if (program_->hasGpuHandle()) {
    return;
  }

  GLuint vertex_shader_pointer = loadShader(GL_VERTEX_SHADER, program_->vertexShaderSource);
  GLuint fragment_shader_pointer = loadShader(GL_FRAGMENT_SHADER, program_->fragmentShaderSource);

  program_->gpuHandle = ::glCreateProgram();
  ::glAttachShader(program_->gpuHandle, vertex_shader_pointer);
  ::glAttachShader(program_->gpuHandle, fragment_shader_pointer);

  // Use the same attribute slots for every program sharing the mesh layout.
  ::glBindAttribLocation(program_->gpuHandle, 0, "aPosition");
  ::glBindAttribLocation(program_->gpuHandle, 1, "aNormal");
  ::glBindAttribLocation(program_->gpuHandle, 2, "aTexCoord0");
  ::glLinkProgram(program_->gpuHandle);

  ::glUseProgram(program_->gpuHandle);

  GLint number_of_uniforms;
  ::glGetProgramiv(program_->gpuHandle, GL_ACTIVE_UNIFORMS, &number_of_uniforms);

  GLchar buffer[128];
  GLsizei name_length;
  GLenum type;
  GLint uniform_size;

  for (int i = 0; i < number_of_uniforms; ++i) {
    ::memset(buffer, '\0', 128);
    ::glGetActiveUniform(program_->gpuHandle, i, 128, &name_length, &uniform_size, &type, buffer);

    int uniform_handle = ::glGetUniformLocation(program_->gpuHandle, buffer);
    assert(uniform_handle >= 0);
    program_->saveUniformHandle(buffer, uniform_handle); //!!! dafuq?
  }

  getShaderHandles(*program_);
}

void Glade::Renderer::moveAllObjectsIntoVideoMemory(void) {
  std::vector<GladeObject *>::iterator i = sceneObjects.begin();

  while (i != sceneObjects.end()) {
    moveIntoVideoMemory(**i);
    i++;
  }

  checkGlError();
}

void Glade::Renderer::moveIntoVideoMemory(GladeObject &scene_object) {
  GladeObject::DrawablesI di = scene_object.getDrawables()->begin();

  while (di != scene_object.getDrawables()->end()) {
    compileShaderProgram((*di)->getShaderProgram().get());
    getShaderHandles(*(*di)->getShaderProgram());
    moveIntoVideoMemory((*di)->getMesh());
    // textures are moved lazily right before drawing them
    di++;
  }

  ::glReleaseShaderCompiler();
}

void Glade::Renderer::moveIntoVideoMemory(std::shared_ptr<Mesh> mesh) {
  if (nullptr == mesh || mesh->hasVideoBufferHandle()) {
    if (nullptr == mesh) {
      ::log("No mesh");
      return;
    }

    //if (mesh->hasVideoBufferHandle())
    //  log("WARNING: Attempting to load mesh which already has a VBO handle, will reload it!");
  }

  // Creating VBOs

  if (!mesh->hasVideoBufferHandle()) {
    GLuint vbo_ids[2];
    ::glGenBuffers(2, vbo_ids);
    mesh->vertexVboId = vbo_ids[0];
    mesh->indexVboId = vbo_ids[1];
    ::log("Created VBOs %d, %d", mesh->vertexVboId, mesh->indexVboId);
  }

  // Creating VAO

  if (mesh->vaoId == -1) {
    GLuint vao_ids[1];
    ::glGenVertexArrays(1, vao_ids);
    mesh->vaoId = vao_ids[0];
    ::log("Created VAO %d", mesh->vaoId);
  }

  ::glBindVertexArray(mesh->vaoId);
  bindBuffers(mesh.get());
  ::glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh->getVertexBufferSize(), mesh->getVertices(),
                 GL_DYNAMIC_DRAW);
  ::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * mesh->getIndexBufferSize(),
                 mesh->getIndices(), GL_DYNAMIC_DRAW);

  if (aPosition >= 0) {
    ::glEnableVertexAttribArray(aPosition);
    ::glVertexAttribPointer(aPosition, POS_SIZE_FLOATS, GL_FLOAT, GL_FALSE, VERTEX_STRIDE_BYTES,
                            POS_OFFSET_BYTES);
  }

  if (aNormal >= 0) {
    ::glEnableVertexAttribArray(aNormal);
    ::glVertexAttribPointer(aNormal, NORMAL_SIZE_FLOATS, GL_FLOAT, GL_FALSE, VERTEX_STRIDE_BYTES,
                            NORMAL_OFFSET_BYTES);
  }

  if (aTexCoord >= 0) {
    ::glEnableVertexAttribArray(aTexCoord);
    ::glVertexAttribPointer(aTexCoord, TEXCOORD_SIZE_FLOATS, GL_FLOAT, GL_FALSE,
                            VERTEX_STRIDE_BYTES, TEXCOORD_OFFSET_BYTES);
  }

  ::glBindVertexArray(0);
  ::glBindBuffer(GL_ARRAY_BUFFER, 0);

  //mesh->erase();
}

void Glade::Renderer::moveTextureIntoVideoMemory(std::shared_ptr<Texture> texture_ptr) {
  Texture *texture = texture_ptr.get();

  if (nullptr == texture) {
    return;
  }

  if (texture->hasVideoBufferHandle()) {
    return;
  }

  if (NULL == texture->getData()) {
    return;
  }

  ::log("Loading texture into video memory");
  GLvoid *data = (GLvoid *)texture->getData();

  GLuint tex_ids[1];
  ::glGenTextures(1, tex_ids);
  texture->setVideoBufferHandle(tex_ids[0]);

  ::glBindTexture(GL_TEXTURE_2D, texture->getVideoBufferHandle());

  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  if (texture->isTextureSizePowerOfTwo()) {
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  } else {
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  ::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->textureWidth, texture->textureHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);

  texture->erase();
}

void Glade::Renderer::removeFromVideoMemory(Drawable &drawable) {
  std::shared_ptr<Mesh> mesh = drawable.getMesh();

  if (const auto *animator = drawable.getSpriteAnimator()) {
    const auto &texture = animator->getAtlas()->getTexture();

    if (texture->hasVideoBufferHandle()) {
      const GLuint id = texture->getVideoBufferHandle();
      ::glDeleteTextures(1, &id);
      texture->setVideoBufferHandle(-1);
    }
  }

  Drawable::Shader2DTextureUniformsCI tex2di = drawable.texture2DUniformsBegin();

  while (tex2di != drawable.texture2DUniformsEnd()) {
    GLuint tex_ids[1];
    tex_ids[0] = tex2di->second->getVideoBufferHandle();
    ::glDeleteTextures(1, tex_ids);
    tex2di->second->setVideoBufferHandle(-1);
    ++tex2di;
  }

  if (mesh->hasVideoBufferHandle()) {
    GLuint vbo_ids[2];
    vbo_ids[0] = mesh->vertexVboId;
    vbo_ids[1] = mesh->indexVboId;
    ::glDeleteBuffers(2, vbo_ids);
    mesh->vertexVboId = -1;
    mesh->indexVboId = -1;
  }
}

void Glade::Renderer::removeAllObjectsFromVideoMemory(void) {
  std::vector<GladeObject *>::iterator i = sceneObjects.begin();

  while (i != sceneObjects.end()) {
    GladeObject::Drawables *drawables = (*i)->getDrawables();
    GladeObject::DrawablesI di = drawables->begin();

    while (di != drawables->end()) {
      removeFromVideoMemory(**di);
      di++;
    }

    i++;
  }
}

void Glade::Renderer::draw(GladeObject::DrawablesI di, Transform &transform) {
  if (::first_cycle) {
    ::log("Started drawing");
    checkGlError();
  }

  if (!perception) {
    ::log("No perception - no drawing");
    return;
  }

  if (const auto *animator = (*di)->getSpriteAnimator(); animator && !animator->hasFrame()) return;

  std::shared_ptr<ShaderProgram> program = (*di)->getShaderProgram();

  if (nullptr == program) {
    ::log("Could not render a drawable: no GPU program");
    return;
  }

  if (program->gpuHandle < 0) {
    ::log("Could not render a drawable: GPU program was not loaded to video memory");
    return;
  }

  ::glUseProgram(program->gpuHandle);
  getShaderHandles(*program);

  if (::first_cycle) {
    checkGlError();
  }

  static float world_matrix[16];

  transform.getMatrix(world_matrix);
  Matrix::multiplyMm(worldViewMatrix, 0, viewMatrix, 0, world_matrix, 0);

  ::glUniformMatrix4fv(uWorldMatrix, 1, GL_FALSE, world_matrix);
  ::glUniformMatrix4fv(uProjectionMatrix, 1, GL_FALSE, projectionMatrix);
  ::glUniformMatrix4fv(uWorldViewMatrix, 1, GL_FALSE, worldViewMatrix);
  ::glUniformMatrix4fv(uCameraTransform, 1, GL_FALSE, cameraTransform);
  ::glUniform3f(uCameraPosition, getCamera()->position->x, getCamera()->position->y,
                getCamera()->position->z);

  if (::first_cycle) {
    checkGlError();
  }

  std::shared_ptr<Glade::Mesh> mesh = (*di)->getMesh();
  ::glBindVertexArray(mesh->vaoId);
  bindBuffers(mesh.get());

  if (::first_cycle) {
    checkGlError();
  }

  if (aPosition >= 0) {
    ::glVertexAttribPointer(aPosition, POS_SIZE_FLOATS, GL_FLOAT, GL_FALSE, VERTEX_STRIDE_BYTES,
                            (const GLvoid *)POS_OFFSET_BYTES);
  }

  if (::first_cycle) {
    checkGlError();
  }

  if (aPosition >= 0) ::glEnableVertexAttribArray(aPosition);

  if (::first_cycle) {
    checkGlError();
  }

  if (aNormal >= 0) {
    ::glVertexAttribPointer(aNormal, NORMAL_SIZE_FLOATS, GL_FLOAT, GL_FALSE, VERTEX_STRIDE_BYTES,
                            (const GLvoid *)NORMAL_OFFSET_BYTES);

    ::glEnableVertexAttribArray(aNormal);
  }

  if (::first_cycle) {
    checkGlError();
  }

  if (aTexCoord >= 0) {
    ::glVertexAttribPointer(aTexCoord, TEXCOORD_SIZE_FLOATS, GL_FLOAT, GL_FALSE,
                            VERTEX_STRIDE_BYTES, (const GLvoid *)TEXCOORD_OFFSET_BYTES);
  }

  if (::first_cycle) {
    checkGlError();
  }

  if (aTexCoord >= 0) ::glEnableVertexAttribArray(aTexCoord);
  perception->adjust();
  writeUniformsToVideoMemory(perception, *program);
  writeUniformsToVideoMemory(*di, *program);

  if (::first_cycle) {
    checkGlError();
  }

  ::glDrawElements(GL_TRIANGLES, (*di)->getMesh()->getIndexBufferSize(), GL_UNSIGNED_INT, 0);

  if (::first_cycle) {
    checkGlError();
  }

  if (aPosition >= 0) ::glDisableVertexAttribArray(aPosition);
  if (aNormal >= 0) ::glDisableVertexAttribArray(aNormal);
  if (aTexCoord >= 0) ::glDisableVertexAttribArray(aTexCoord);

  if (::first_cycle) {
    checkGlError();
  }

  ::glBindVertexArray(0);
}

void Glade::Renderer::setTexture2DUniformToShader(const std::string &uniform_name,
                                                  GLuint texture_video_buffer_handle,
                                                  unsigned texture_unit_number,
                                                  ShaderProgram &program_) {
  if (!texture_video_buffer_handle) return;

  ::glActiveTexture(GL_TEXTURE0 + texture_unit_number);
  ::glBindTexture(GL_TEXTURE_2D, texture_video_buffer_handle);

  if (::first_cycle) {
    checkGlError();
  }

  GLuint location = ::glGetUniformLocation(program_.gpuHandle, uniform_name.c_str());
  ::glUniform1i(location, texture_unit_number);

  if (::first_cycle) {
    checkGlError();
  }
}

void Glade::Renderer::writeUniformsToVideoMemory(Drawable *drawable, ShaderProgram &program_) {
  if (drawable == NULL) return;

  Drawable::ShaderFloatUniformsCI fi = drawable->floatUniformsBegin();
  GLint uniform_handle = -1;

  while (fi != drawable->floatUniformsEnd()) {
    uniform_handle = program_.getUniformHandle(fi->first);
    ::glUniform1f(uniform_handle, fi->second);
    ++fi;
  }

  Drawable::ShaderBoolUniformsCI bi = drawable->boolUniformsBegin();

  while (bi != drawable->boolUniformsEnd()) {
    uniform_handle = program_.getUniformHandle(bi->first);
    ::glUniform1i(uniform_handle, bi->second ? 1 : 0);
    ++bi;
  }

  Drawable::ShaderIntUniformsCI ii = drawable->intUniformsBegin();

  while (ii != drawable->intUniformsEnd()) {
    uniform_handle = program_.getUniformHandle(ii->first);
    ::glUniform1i(uniform_handle, ii->second);
    ++ii;
  }

  Drawable::ShaderVec3UniformsCI v3i = drawable->vec3UniformsBegin();

  while (v3i != drawable->vec3UniformsEnd()) {
    uniform_handle = program_.getUniformHandle(v3i->first);
    ::glUniform3f(uniform_handle, v3i->second.x, v3i->second.y, v3i->second.z);
    ++v3i;
  }

  Drawable::ShaderVec4UniformsCI v4i = drawable->vec4UniformsBegin();

  while (v4i != drawable->vec4UniformsEnd()) {
    uniform_handle = program_.getUniformHandle(v4i->first);
    ::glUniform4f(uniform_handle, v4i->second.x, v4i->second.y, v4i->second.z, v4i->second.w);
    ++v4i;
  }

  const auto *animator = drawable->getSpriteAnimator();
  const auto *uv = drawable->getTexture2DTransform();
  const GLint geometry = ::glGetUniformLocation(program_.gpuHandle, "uSpriteGeometry");
  const GLint scale_x = ::glGetUniformLocation(program_.gpuHandle, "uTexCoordScaleX0");
  const GLint scale_y = ::glGetUniformLocation(program_.gpuHandle, "uTexCoordScaleY0");
  const GLint offset_x = ::glGetUniformLocation(program_.gpuHandle, "uTexCoordOffsetX0");
  const GLint offset_y = ::glGetUniformLocation(program_.gpuHandle, "uTexCoordOffsetY0");
  // Always reset geometry for ordinary drawables sharing a sprite shader.
  ::glUniform4f(geometry, 1, 1, 0, 0);

  if (animator) {
    if (!animator->hasFrame()) return; // draw() skips sprites without a selection.

    if (geometry < 0 || scale_x < 0 || scale_y < 0 || offset_x < 0 || offset_y < 0 ||
        ::glGetUniformLocation(program_.gpuHandle, "uTextureSampler0") < 0 ||
        dynamic_cast<Glade::Rectangle *>(drawable->getMesh().get()) == nullptr)
      throw GladeException("Sprite requires Rectangle and texcoord_frames/textured shaders");

    const auto &frame = animator->getCurrentFrame();
    const auto &texture = animator->getAtlas()->getTexture();
    const bool flip_x = uv->isFlippedHorizontally(), flip_y = uv->isFlippedVertically();
    // Keep geometry winding unchanged when mirroring: reflect its centre about
    // the pivot, and reverse UV direction within the selected atlas rectangle.
    ::glUniform4f(geometry, frame.width * 0.5f, frame.height * 0.5f,
                  (frame.width * 0.5f - frame.pivotX) * (flip_x ? -1 : 1),
                  (frame.pivotY - frame.height * 0.5f) * (flip_y ? -1 : 1));
    ::glUniform1f(scale_x, float(frame.width) / texture->textureWidth * (flip_x ? -1 : 1));
    ::glUniform1f(scale_y, float(frame.height) / texture->textureHeight * (flip_y ? 1 : -1));
    ::glUniform1f(offset_x, float(frame.x + (flip_x ? frame.width : 0)) / texture->textureWidth);
    ::glUniform1f(offset_y, float(frame.y + (flip_y ? 0 : frame.height)) / texture->textureHeight);
    if (!texture->hasVideoBufferHandle()) moveTextureIntoVideoMemory(texture);
    setTexture2DUniformToShader("uTextureSampler0", texture->getVideoBufferHandle(), 0, program_);

    return; // Atlas texture is the sole texture source for sprite rendering.
  }

  ::glUniform1f(scale_x, uv->textureScaleX);
  ::glUniform1f(scale_y, uv->textureScaleY);
  ::glUniform1f(offset_x, uv->textureOffsetX + (uv->isFlippedHorizontally() ? 1.0f : 0.0f));
  ::glUniform1f(offset_y, uv->textureOffsetY + (uv->isFlippedVertically() ? 1.0f : 0.0f));

  Drawable::Shader2DTextureUniformsCI tex2di = drawable->texture2DUniformsBegin();

  unsigned i = 0;

  while (tex2di != drawable->texture2DUniformsEnd()) {
    uniform_handle = program_.getUniformHandle(tex2di->first);
    std::shared_ptr<Texture> texture = tex2di->second;

    if (!texture->hasVideoBufferHandle()) {
      moveTextureIntoVideoMemory(texture);
    }

    setTexture2DUniformToShader(tex2di->first, texture->getVideoBufferHandle(), i, program_);
    ++i;
    ++tex2di;
  }
}

void Glade::Renderer::bindBuffers(Mesh *mesh) {
  if (!mesh) {
    ::glBindBuffer(GL_ARRAY_BUFFER, 0);
    ::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return;
  }

  if (!mesh->hasVideoBufferHandle()) {
    ::log("Render error: trying to use a mesh which was not loaded into video buffer");
  }

  ::glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexVboId);
  ::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexVboId);
}

void Glade::Renderer::switchProjectionMode(ProjectionMode projection_mode, bool force) {
  if (force || projection_mode != currentProjectionMode) {
    switch (projection_mode) {
    case ORTHO:
      Matrix::orthoM(projectionMatrix, 0, -aspectRatio, aspectRatio, -1, 1,
                     -(float)::DOUBLE_CUBE_DIAGONAL, (float)::DOUBLE_CUBE_DIAGONAL);
      ::glDisable(GL_DEPTH_TEST);
      break;
    case PERSPECTIVE:
      Matrix::perspectiveM(projectionMatrix, 0, 45, aspectRatio, 0.1, 150);
      ::glEnable(GL_DEPTH_TEST);
      break;
    default:;
      //throw new RuntimeException("Unexpected projection mode. Use ORTHO or PERSPECTIVE constants");
    }

    currentProjectionMode = projection_mode;
  }
}

void Glade::Renderer::switchProjectionMode(ProjectionMode projection_mode) {
  switchProjectionMode(projection_mode, false);
}

void Glade::Renderer::getShaderHandles(ShaderProgram &program_) {
  uProjectionMatrix = ::glGetUniformLocation(program_.gpuHandle, "uProjectionMatrix");
  uWorldViewMatrix = ::glGetUniformLocation(program_.gpuHandle, "uWorldViewMatrix");
  uWorldMatrix = ::glGetUniformLocation(program_.gpuHandle, "uWorldMatrix");
  uCameraTransform = ::glGetUniformLocation(program_.gpuHandle, "uCameraTransform");
  uCameraPosition = ::glGetUniformLocation(program_.gpuHandle, "uCameraPosition");

  aPosition = ::glGetAttribLocation(program_.gpuHandle, "aPosition");
  aNormal = ::glGetAttribLocation(program_.gpuHandle, "aNormal");
  aTexCoord = ::glGetAttribLocation(program_.gpuHandle, "aTexCoord0");

  uDiffuseMap = ::glGetUniformLocation(program_.gpuHandle, "uDiffuseMap");
  uSpecularMap = ::glGetUniformLocation(program_.gpuHandle, "uSpecularMap");
  uTexScaleX = ::glGetUniformLocation(program_.gpuHandle, "uTexCoordScaleX0");
  uTexScaleY = ::glGetUniformLocation(program_.gpuHandle, "uTexCoordScaleY0");
  uTexOffsetX = ::glGetUniformLocation(program_.gpuHandle, "uTexCoordOffsetX0");
  uTexOffsetY = ::glGetUniformLocation(program_.gpuHandle, "uTexCoordOffsetY0");
}

GLuint Glade::Renderer::loadShader(GLuint shader_type, std::vector<char> &shader_source) {
  GLuint shader_handle = ::glCreateShader(shader_type);

  if (shader_handle) {
    const char *shader_source_line_ptr = shader_source.data();
    GLint shader_source_size = shader_source.size();
    ::glShaderSource(shader_handle, 1, &shader_source_line_ptr, &shader_source_size);
    ::glCompileShader(shader_handle);
    GLint compiled = 0;
    ::glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
      GLint info_len = 0;
      ::glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &info_len);

      if (info_len) {
        char *buf = (char *)::malloc(info_len);

        if (buf) {
          ::glGetShaderInfoLog(shader_handle, info_len, NULL, buf);
          ::log("Shader compilation error:");
          ::log(buf);
          ::free(buf);
          throw GladeException("Shader compilation error");
        }

        ::glDeleteShader(shader_handle);
        shader_handle = 0;
      }
    }
  }

  return shader_handle;
}

int Glade::Renderer::checkGlError(void) {
  int error_code = ::glGetError();

  switch (error_code) {
  case GL_INVALID_ENUM:
    ::log("GL_INVALID_ENUM: A GLenum argument is out of range. The command that generated this "
          "error was ignored.");
    break;
  case GL_INVALID_VALUE:
    ::log("GL_INVALID_VALUE: A numeric argument is out of range. The command that generated this "
          "error was ignored.");
    break;
  case GL_INVALID_OPERATION:
    ::log("GL_INVALID_OPERATION: The command cannot be performed in the current OpenGL state. The "
          "command that generated this error was ignored.");
    break;
  case GL_OUT_OF_MEMORY:
    ::log("GL_OUT_OF_MEMORY: There is insufficient memory to execute this command. The state of "
          "the OpenGL pipeline is undefined after this point.");
    break;
  default:
    ::log("No OpenGL error at this point");
    break;
  }

  return error_code;
}
