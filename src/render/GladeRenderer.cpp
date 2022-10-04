#include <math.h>
#include <assert.h>
#include <algorithm>

#include <GL/glew.h>

#include "glade/debug/log.h"
#include "glade/math/util.h"
#include "glade/math/Matrix.h"
#include "glade/render/GladeRenderer.h"
#include "glade/render/Drawable.h"
#include "glade/render/ShaderProgram.h"
#include "glade/render/Perception.h"
#include "glade/math/Vector.h"

const GLuint  Glade::Renderer::POS_SIZE_FLOATS        = 3;
const GLuint  Glade::Renderer::NORMAL_SIZE_FLOATS     = 3;
const GLuint  Glade::Renderer::TEXCOORD_SIZE_FLOATS   = 2;
const GLuint  Glade::Renderer::POS_OFFSET_FLOATS      = 0;
const GLuint  Glade::Renderer::NORMAL_OFFSET_FLOATS   = POS_SIZE_FLOATS;
const GLuint  Glade::Renderer::TEXCOORD_OFFSET_FLOATS = POS_SIZE_FLOATS + NORMAL_SIZE_FLOATS;
const GLuint  Glade::Renderer::VERTEX_STRIDE_FLOATS   = POS_SIZE_FLOATS + NORMAL_SIZE_FLOATS + TEXCOORD_SIZE_FLOATS;
const GLvoid * Glade::Renderer::POS_OFFSET_BYTES       = (const GLvoid *) (POS_OFFSET_FLOATS      * sizeof(float));
const GLvoid * Glade::Renderer::NORMAL_OFFSET_BYTES    = (const GLvoid *) (NORMAL_OFFSET_FLOATS   * sizeof(float));
const GLvoid * Glade::Renderer::TEXCOORD_OFFSET_BYTES  = (const GLvoid *) (TEXCOORD_OFFSET_FLOATS * sizeof(float));
const GLsizei Glade::Renderer::VERTEX_STRIDE_BYTES    = VERTEX_STRIDE_FLOATS   * sizeof(float);

static bool firstCycle;

Glade::Renderer::Renderer(void):
  perception(NULL),
  initialized(false),
  sceneProjectionMode(PERSPECTIVE)
{}


void Glade::Renderer::onSurfaceCreated()
{
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  //glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
  moveAllObjectsIntoVideoMemory();
  log("Initialized renderer");
  initialized = true;
  checkGLError();
}


void Glade::Renderer::onSurfaceChanged(int width, int height)
{
  viewportWidth = width;
  viewportHeight = height;
  aspectRatio = (float) viewportWidth / viewportHeight;
  glViewport(0, 0, viewportWidth, viewportHeight);
  
  switchProjectionMode(sceneProjectionMode, true);
  checkGLError();
}


void Glade::Renderer::add(GladeObject* pSceneObject)
{
  if (this->initialized) {
    moveIntoVideoMemory(*pSceneObject);
  } else {
    log("Adding scene object, but renderer is not initialised");
  }
  
  sceneObjects.push_back(pSceneObject);
}

void Glade::Renderer::remove(GladeObject *sceneObject)
{
  std::vector<GladeObject*>::iterator oi =
    std::find(sceneObjects.begin(), sceneObjects.end(), sceneObject);
    
  if (oi == sceneObjects.end()) {
    log("Warning: couldn't remove object, because it's not in the renderer");
    return;
  }
  
  GladeObject::Drawables *drawables = (*oi)->getDrawables();
    
  for (GladeObject::DrawablesI di = drawables->begin(); di != drawables->end(); di++) {
    removeFromVideoMemory(**di);
    checkGLError();
    (*di)->getTextureTransform()->removeAnimationCallbacks();
  }
  
  sceneObjects.erase(oi);
}

void Glade::Renderer::clear(void)
{
  log("Clearing renderer");
  firstCycle = true;
  removeAllObjectsFromVideoMemory();

  GladeObject::Drawables* drawables;
  std::vector<GladeObject*>::iterator oi = sceneObjects.begin(); // must be a 'set'

  while (oi != sceneObjects.end()) {
    drawables = (*oi)->getDrawables();
    
    for (GladeObject::DrawablesI di = drawables->begin(); di != drawables->end(); di++) {
      (*di)->getTextureTransform()->removeAnimationCallbacks();
    }
    
    oi = sceneObjects.erase(oi);
  }

  checkGLError();
  
  log("Done clearing renderer");
}

void Glade::Renderer::onDrawFrame(void)
{
  glClearColor(this->backgroundColor.x, this->backgroundColor.y, this->backgroundColor.z, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (getCamera()) {
    getCamera()->getMatrix(cameraTransform);
    getCamera()->getCameraMatrix(viewMatrix);
    switchProjectionMode(sceneProjectionMode);
    drawAll(sceneObjects.begin(), sceneObjects.end());
  } else if (firstCycle) {
    log("Warning: no active camera, cannot render the scene");
  }

  glFlush();
  firstCycle = false;
}

void Glade::Renderer::setSceneProjectionMode(ProjectionMode projectionMode)
{
  this->sceneProjectionMode = projectionMode;
}

int Glade::Renderer::getViewportWidth(void)
{
  return viewportWidth;
}

int Glade::Renderer::getViewportHeight(void)
{
  return viewportHeight;
}

void Glade::Renderer::setBackgroundColor(float r, float g, float b)
{
  this->backgroundColor.set(r, g, b);
}

int Glade::Renderer::percentToPixels(float percent)
{
  return (int)((viewportWidth < viewportHeight ? viewportWidth / 2 : viewportHeight / 2) * percent); 
}

float Glade::Renderer::pixelsToPercent(float pixels)
{
  return pixels / (viewportWidth < viewportHeight ? viewportWidth / 2 : viewportHeight / 2);
}

void Glade::Renderer::moveZeroToUpperLeftCorner(void)
{
  getCamera()->setPosition(getHalfViewportWidthCoords(),  getHalfViewportHeightCoords(), 0);
}

float Glade::Renderer::getHalfViewportWidthCoords(void)
{
  return aspectRatio > 1 ? aspectRatio : 1;
}

float Glade::Renderer::getHalfViewportHeightCoords(void)
{
  return aspectRatio > 1 ? 1 : aspectRatio;
}
  
float Glade::Renderer::getViewportWidthCoords(void)
{
  return 2 * aspectRatio;
}

float Glade::Renderer::getViewportHeightCoords(void)
{
  return 2;
}

Glade::Vector2f Glade::Renderer::getPointCoords(float screenX, float screenY)
{
  Glade::Vector2f result;
  
  result.x = screenX / viewportWidth * 2 * aspectRatio - aspectRatio;
  result.y = screenY / viewportHeight * 2 - 1;
  
  return result;
}

Glade::Vector3f Glade::Renderer::unprojectPoint(float x, float y, float z)
{
  float invViewMatrix[16];
  float invProjMatrix[16];

  Glade::Vector4f point(x, y, z, 1.0);
  Glade::Vector4f resultPoint;

  Matrix::invertM(invViewMatrix, 0, viewMatrix, 0);
  Matrix::invertM(invProjMatrix, 0, projectionMatrix, 0);
  Matrix::multiplyMV(resultPoint, invProjMatrix, point); 
  resultPoint.w = 1.0;
  Matrix::multiplyMV(resultPoint, invViewMatrix, resultPoint); 

  return Glade::Vector3f(resultPoint.x, resultPoint.y, resultPoint.z);
}

void Glade::Renderer::drawAll(std::vector<GladeObject*>::iterator i, std::vector<GladeObject*>::iterator end)
{
  Transform finalWorldTransformForDrawable;
  GladeObject::Drawables* drawables;
  
  while (i != end) {
    if ((*i)->isEnabled()) {
      drawables = (*i)->getDrawables();
    
      for (GladeObject::DrawablesI di = drawables->begin(); di != drawables->end(); di++) {
        if ((*i)->isViewEnabled(*di)) {
          (*i)->getTransform()->combineWith(
            *(*di)->getTransform(),
            (*di)->preservePosition,
            (*di)->preserveRotation,
            (*di)->preserveScale,
            finalWorldTransformForDrawable
          );
          
          draw(di, finalWorldTransformForDrawable);
        }
      }
    }
    
    i++;
  }
}

void Glade::Renderer::compileShaderProgram(ShaderProgram *program)
{
  
  if (program == nullptr || program == NULL) {
    log("Warning: tried to compile a shader program, but it's null");
  }
  
  if (program->hasGpuHandle()) {
    return;
  }
    
  GLuint vertexShaderPointer = loadShader(GL_VERTEX_SHADER, program->vertexShaderSource);
  GLuint fragmentShaderPointer = loadShader(GL_FRAGMENT_SHADER, program->fragmentShaderSource);
  
  program->gpuHandle = glCreateProgram();
  glAttachShader(program->gpuHandle, vertexShaderPointer);
  glAttachShader(program->gpuHandle, fragmentShaderPointer);
  glLinkProgram(program->gpuHandle);

  glUseProgram(program->gpuHandle);
 
  GLint numberOfUniforms;
  glGetProgramiv(program->gpuHandle, GL_ACTIVE_UNIFORMS, &numberOfUniforms);
  
  GLchar buffer[128];
  GLsizei nameLength;
  GLenum type;
  GLint uniformSize;
  
  for (int i = 0; i < numberOfUniforms; ++i) {
    memset(buffer, '\0', 128);
    glGetActiveUniform(program->gpuHandle, i, 128, &nameLength, &uniformSize, &type, buffer);
    
    int uniformHandle = glGetUniformLocation(program->gpuHandle, buffer);
    assert(uniformHandle >= 0);
    program->saveUniformHandle(buffer, uniformHandle); //!!!
  }

  getShaderHandles(*program);
}


void Glade::Renderer::moveAllObjectsIntoVideoMemory(void)
{
  std::vector<GladeObject*>::iterator i = sceneObjects.begin();
  
  while (i != sceneObjects.end()) {
    moveIntoVideoMemory(**i);
    i++;
  }
  
  checkGLError();
}

void Glade::Renderer::moveIntoVideoMemory(GladeObject &sceneObject)
{
  GladeObject::DrawablesI di = sceneObject.getDrawables()->begin();

  while (di != sceneObject.getDrawables()->end()) {
    moveIntoVideoMemory((*di)->getMesh());
    moveIntoVideoMemory((*di)->getTexture());
    compileShaderProgram((*di)->getShaderProgram().get());
    di++;
  }
  
  glReleaseShaderCompiler();
}

void Glade::Renderer::moveIntoVideoMemory(std::shared_ptr<Mesh> mesh)
{
  if (nullptr == mesh || mesh->hasVideoBufferHandle()) {
    if (nullptr == mesh) {
      log("No mesh");
      return;
    }

    //if (mesh->hasVideoBufferHandle())
    //  log("WARNING: Attempting to load mesh which already has a VBO handle, will reload it!");
  }

  // Creating VBOs
  if (!mesh->hasVideoBufferHandle()) {
    GLuint vboIds[2];
    glGenBuffers(2, vboIds);
    mesh->vertexVboId = vboIds[0];
    mesh->indexVboId = vboIds[1];
    log("Created VBOs %d, %d", mesh->vertexVboId, mesh->indexVboId);
  }

  // Loading data into VBOs
  bindBuffers(mesh.get());
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh->getVertexBufferSize(), mesh->getVertices(), GL_DYNAMIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * mesh->getIndexBufferSize(), mesh->getIndices(), GL_DYNAMIC_DRAW);
  bindBuffers(0);

  // Creating VAO
  if (mesh->vaoId == -1) {
    GLuint vaoIds[1];
    glGenVertexArrays(1, vaoIds);
    mesh->vaoId = vaoIds[0];
    log("Created VAO %d", mesh->vaoId);
  }

  glBindVertexArray(mesh->vaoId);

  glEnableVertexAttribArray(aPosition);
  glEnableVertexAttribArray(aNormal);
  glEnableVertexAttribArray(aTexCoord);
 
  glVertexAttribPointer(
    aPosition, POS_SIZE_FLOATS,
    GL_FLOAT, GL_FALSE,
    VERTEX_STRIDE_BYTES, (const GLvoid *) POS_OFFSET_BYTES
  );

  glVertexAttribPointer(
    aNormal, NORMAL_SIZE_FLOATS,
    GL_FLOAT, GL_FALSE,
    VERTEX_STRIDE_BYTES, (const GLvoid *) NORMAL_OFFSET_BYTES
  );

  glVertexAttribPointer(
    aTexCoord, TEXCOORD_SIZE_FLOATS,
    GL_FLOAT, GL_FALSE,
    VERTEX_STRIDE_BYTES, (const GLvoid*) TEXCOORD_OFFSET_BYTES
  );

  bindBuffers(mesh.get());

  glBindVertexArray(0);
  bindBuffers(0);

  //mesh->erase();
}

void Glade::Renderer::moveIntoVideoMemory(std::shared_ptr<Texture> texturePtr)
{
  Texture *texture = texturePtr.get();
  
  if (nullptr == texture) {
    return;
  }
  
  if (texture->hasVideoBufferHandle()) {
    return;
  }
  
  if (NULL == texture->getData()) {
    return;
  }
  
  log("Loading texture into video memory");
  GLvoid* data = (GLvoid*) texture->getData();
  
  GLuint texIds[1];
  glGenTextures(1, texIds);
  texture->setVideoBufferHandle(texIds[0]);
  
  glBindTexture(GL_TEXTURE_2D, texture->getVideoBufferHandle());
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  if (texture->isTextureSizePowerOfTwo()) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
  }
  
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGBA,
    texture->textureWidth,
    texture->textureHeight,
    0,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
    data
  );
  
  texture->erase();
}

void Glade::Renderer::removeFromVideoMemory(Drawable &drawable)
{
  std::shared_ptr<Texture> texture = drawable.getTexture();
  std::shared_ptr<Mesh> mesh = drawable.getMesh();
  
  if (nullptr != texture && texture->hasVideoBufferHandle()) {
    GLuint texIds[1];
    texIds[0] = texture->getVideoBufferHandle();
    glDeleteTextures(1, texIds);
    texture->setVideoBufferHandle(-1);
  }
  
  if (mesh->hasVideoBufferHandle()) {
    GLuint vboIds[2];
    vboIds[0] = mesh->vertexVboId;
    vboIds[1] = mesh->indexVboId;
    glDeleteBuffers(2, vboIds);
    mesh->vertexVboId = -1;
    mesh->indexVboId = -1;
  }
}

void Glade::Renderer::removeAllObjectsFromVideoMemory(void)
{
  std::vector<GladeObject*>::iterator i = sceneObjects.begin();
  
  while (i != sceneObjects.end()) {
    GladeObject::Drawables* drawables = (*i)->getDrawables();
    GladeObject::DrawablesI di = drawables->begin();
    
    while (di != drawables->end()) {
      removeFromVideoMemory(**di);
      di++;
    }
    
    i++;
  }
}

void Glade::Renderer::draw(GladeObject::DrawablesI di, Transform &transform)
{
  if (firstCycle) {
    log("Started drawing");
    checkGLError();
  }
 
  if (!perception) {
    log("No perception - no drawing");
    return;
  }

  std::shared_ptr<ShaderProgram> program = (*di)->getShaderProgram();
  
  if (nullptr == program) {
    log("Could not render a drawable: no GPU program");
    return;
  }
  
  if (program->gpuHandle < 0) {
    log("Could not render a drawable: GPU program was not loaded to video memory");
    return;
  }
  
  glUseProgram(program->gpuHandle);
  getShaderHandles(*program);

  if (firstCycle) {
    checkGLError();
  }
  
  static float worldMatrix[16];

  transform.getMatrix(worldMatrix);
  Matrix::multiplyMM(worldViewMatrix, 0, viewMatrix, 0, worldMatrix, 0);

  glUniformMatrix4fv(uWorldMatrix, 1, GL_FALSE, worldMatrix);
  glUniformMatrix4fv(uProjectionMatrix, 1, GL_FALSE, projectionMatrix);
  glUniformMatrix4fv(uWorldViewMatrix, 1, GL_FALSE, worldViewMatrix);
  glUniformMatrix4fv(uCameraTransform, 1, GL_FALSE, cameraTransform);
  glUniform3f(uCameraPosition, getCamera()->position->x, getCamera()->position->y, getCamera()->position->z);
  
  if (firstCycle) {
    checkGLError();
  }
  
  std::shared_ptr<Glade::Mesh> mesh = (*di)->getMesh();
  bindBuffers(mesh.get());
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexVboId);
  glBindVertexArray(mesh->vaoId);
  
  if (firstCycle) {
    checkGLError();
  }

  glVertexAttribPointer(
    aPosition, POS_SIZE_FLOATS,
    GL_FLOAT, GL_FALSE,
    VERTEX_STRIDE_BYTES, (const GLvoid *) POS_OFFSET_BYTES
  );

  if (firstCycle) {
    checkGLError();
  }

  glEnableVertexAttribArray(aPosition);
  
  if (firstCycle) {
    checkGLError();
  }

  if (aNormal >= 0) {
    glVertexAttribPointer(
      aNormal, NORMAL_SIZE_FLOATS,
      GL_FLOAT, GL_FALSE,
      VERTEX_STRIDE_BYTES, (const GLvoid *) NORMAL_OFFSET_BYTES
    );

    glEnableVertexAttribArray(aNormal);
  }

  if (firstCycle) {
    checkGLError();
  }

  std::shared_ptr<Texture> texture = (*di)->getTexture();
  
  if (texture != nullptr) {
    if (texture->hasVideoBufferHandle()) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture->getVideoBufferHandle());

      if (firstCycle) {
        checkGLError();
      }

      TextureTransform* texTransform = (*di)->getTextureTransform();
      texTransform->executeCallbacks();

      glUniform1i(uSamplerNumber, 0);
      glUniform1f(uTexScaleX, texTransform->textureScaleX  / (float) texture->numberOfFrames * texTransform->getTextureScaleXModifierForFrame(*texture));
      glUniform1f(uTexScaleY, texTransform->textureScaleY  / (float) texture->numberOfAnimations * texTransform->getTextureScaleYModifierForFrame(*texture));
      glUniform1f(uTexOffsetX, texTransform->getCurrentFrameNumber(*texture) * texture->texCoordFrameWidth);
      glUniform1f(uTexOffsetY, texTransform->getCurrentAnimationNumber(*texture) * texture->texCoordFrameHeight);

      if (firstCycle) {
        checkGLError();
      }

      glVertexAttribPointer(
        aTexCoord, TEXCOORD_SIZE_FLOATS,
        GL_FLOAT, GL_FALSE,
        VERTEX_STRIDE_BYTES, (const GLvoid*) TEXCOORD_OFFSET_BYTES
      );

      if (firstCycle) {
        checkGLError();
      }
      
      glEnableVertexAttribArray(aTexCoord);
    }
  }

  // Always enable for now (as we use texCoord for debugging color)
  // ---------
  glVertexAttribPointer(
    aTexCoord, TEXCOORD_SIZE_FLOATS,
    GL_FLOAT, GL_FALSE,
    VERTEX_STRIDE_BYTES, (const GLvoid*) TEXCOORD_OFFSET_BYTES
  );

  if (firstCycle) {
    checkGLError();
  }
  
  glEnableVertexAttribArray(aTexCoord);
  // ---------

  perception->adjust();
  writeUniformsToVideoMemory(perception, *program);
  writeUniformsToVideoMemory(*di, *program);

  if (firstCycle) {
    checkGLError();
  }

  glDrawElements(GL_TRIANGLES, (*di)->getMesh()->getIndexBufferSize(), GL_UNSIGNED_SHORT, 0);
  
  if (firstCycle) {
    checkGLError();
  }

  //glDisableVertexAttribArray(aPosition);
  //glDisableVertexAttribArray(aNormal);

  //if (texture != nullptr && texture->hasVideoBufferHandle()) {
    //glDisableVertexAttribArray(aTexCoord);
  //}

  if (firstCycle) {
    checkGLError();
  }

  glBindVertexArray(0);
}

void Glade::Renderer::writeUniformsToVideoMemory(Drawable *drawable, ShaderProgram &program)
{
  if (drawable == NULL)
    return;

  Drawable::ShaderFloatUniformsCI fi = drawable->floatUniformsBegin();
  GLint uniformHandle = -1;
  
  while (fi != drawable->floatUniformsEnd()) {
    uniformHandle = program.getUniformHandle(fi->first);
    glUniform1f(uniformHandle, fi->second);
    ++fi;
  }

  Drawable::ShaderBoolUniformsCI bi = drawable->boolUniformsBegin();
  
  while (bi != drawable->boolUniformsEnd()) {
    uniformHandle = program.getUniformHandle(bi->first);
    glUniform1i(uniformHandle, bi->second ? 1 : 0);
    ++bi;
  }

  Drawable::ShaderIntUniformsCI ii = drawable->intUniformsBegin();
  
  while (ii != drawable->intUniformsEnd()) {
    uniformHandle = program.getUniformHandle(ii->first);
    glUniform1i(uniformHandle, ii->second);
    ++ii;
  }

  Drawable::ShaderVec3UniformsCI v3i = drawable->vec3UniformsBegin();

  while (v3i != drawable->vec3UniformsEnd()) {
    uniformHandle = program.getUniformHandle(v3i->first);
    glUniform3f(uniformHandle, v3i->second.x, v3i->second.y, v3i->second.z);
    ++v3i;
  }
  
  Drawable::ShaderVec4UniformsCI v4i = drawable->vec4UniformsBegin();
  
  while (v4i != drawable->vec4UniformsEnd()) {
    uniformHandle = program.getUniformHandle(v4i->first);
    glUniform4f(uniformHandle, v4i->second.x, v4i->second.y, v4i->second.z, v4i->second.w);
    ++v4i;
  }
}

void Glade::Renderer::bindBuffers(Mesh *mesh)
{
  if (!mesh) {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return;
  }

  if (!mesh->hasVideoBufferHandle()) {
    log("Render error: trying to use a mesh which was not loaded into video buffer");
  }
  
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexVboId);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexVboId);
}

void Glade::Renderer::switchProjectionMode(ProjectionMode projectionMode, bool force)
{
  if (force || projectionMode != currentProjectionMode) {
    switch (projectionMode) {
      case ORTHO:
        Matrix::orthoM(projectionMatrix, 0, -aspectRatio, aspectRatio, -1, 1, -(float)DOUBLE_CUBE_DIAGONAL, (float)DOUBLE_CUBE_DIAGONAL);
        glDisable(GL_DEPTH_TEST);
        break;
      case PERSPECTIVE:
        Matrix::perspectiveM(projectionMatrix, 0, 45, aspectRatio, 1, 150);
        glEnable(GL_DEPTH_TEST);
        break;
      default: ;
        //throw new RuntimeException("Unexpected projection mode. Use ORTHO or PERSPECTIVE constants");
    }
    
    currentProjectionMode = projectionMode;
  }
}

void Glade::Renderer::switchProjectionMode(ProjectionMode projectionMode)
{
  switchProjectionMode(projectionMode, false);
}

void Glade::Renderer::getShaderHandles(ShaderProgram &program)
{
  uProjectionMatrix   = glGetUniformLocation(program.gpuHandle, "uProjectionMatrix");
  uWorldViewMatrix  = glGetUniformLocation(program.gpuHandle, "uWorldViewMatrix");
  uWorldMatrix  = glGetUniformLocation(program.gpuHandle, "uWorldMatrix");
  uCameraTransform = glGetUniformLocation(program.gpuHandle, "uCameraTransform");
  uCameraPosition = glGetUniformLocation(program.gpuHandle, "uCameraPosition");

  aPosition      = glGetAttribLocation(program.gpuHandle, "aPosition");
  aNormal        = glGetAttribLocation(program.gpuHandle, "aNormal");
  aTexCoord      = glGetAttribLocation(program.gpuHandle, "aTexCoord0");
  
  uSamplerNumber  = glGetUniformLocation(program.gpuHandle, "uTextureSampler0");
  uTexScaleX       = glGetUniformLocation(program.gpuHandle, "uTexCoordScaleX0");
  uTexScaleY       = glGetUniformLocation(program.gpuHandle, "uTexCoordScaleY0");
  uTexOffsetX     = glGetUniformLocation(program.gpuHandle, "uTexCoordOffsetX0");
  uTexOffsetY     = glGetUniformLocation(program.gpuHandle, "uTexCoordOffsetY0");
} 

GLuint Glade::Renderer::loadShader(GLuint shaderType, std::vector<char> &shader_source)
{
  GLuint shaderHandle = glCreateShader(shaderType);
  
  if (shaderHandle) {
    const char *shaderSourceLinePtr = shader_source.data();
    GLint shaderSourceSize = shader_source.size();
    glShaderSource(shaderHandle, 1, &shaderSourceLinePtr, &shaderSourceSize);
    glCompileShader(shaderHandle);
    GLint compiled = 0;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compiled);
    
    if (!compiled) {
      GLint infoLen = 0;
      glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &infoLen);
      
      if (infoLen) {
        char* buf = (char*) malloc(infoLen);
        
        if (buf) {
            glGetShaderInfoLog(shaderHandle, infoLen, NULL, buf);
            log("Shader compilation error:");
            log(buf);
            free(buf);
            throw GladeException("Shader compilation error");
        }
        
        glDeleteShader(shaderHandle);
        shaderHandle = 0;
      }
    }
  }

  return shaderHandle;
}

int Glade::Renderer::checkGLError(void)
{
  int errorCode = glGetError();
  
  switch (errorCode) {
    case GL_INVALID_ENUM:
      log("GL_INVALID_ENUM: A GLenum argument is out of range. The command that generated this error was ignored.");
      break;
    case GL_INVALID_VALUE:
      log("GL_INVALID_VALUE: A numeric argument is out of range. The command that generated this error was ignored.");
      break;
    case GL_INVALID_OPERATION:
      log("GL_INVALID_OPERATION: The command cannot be performed in the current OpenGL state. The command that generated this error was ignored.");
      break;
    case GL_OUT_OF_MEMORY:
      log("GL_OUT_OF_MEMORY: There is insufficient memory to execute this command. The state of the OpenGL pipeline is undefined after this point.");
      break;
    default:
      log("No OpenGL error at this point");
      break;
  }
  
  return errorCode;
}

