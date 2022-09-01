#pragma once

#include <vector>
#include <memory>
#include <set>

#include <GL/glew.h>

#include "glade/render/Perception.h"
#include "glade/math/Transform.h"
#include "glade/GladeObject.h"

class DrawFrameHook;
class ShaderProgram;
class Texture;
class Perception;

namespace Glade {
  class Mesh;
  class Vector3f;

  class Renderer {
  public:
    enum ProjectionMode {PERSPECTIVE, ORTHO};
    
    static const GLuint POS_SIZE_FLOATS;
    static const GLuint COLOR_SIZE_FLOATS;
    static const GLuint NORMAL_SIZE_FLOATS;
    static const GLuint TEXCOORD_SIZE_FLOATS;
    static const GLuint POS_OFFSET_FLOATS;
    static const GLuint COLOR_OFFSET_FLOATS;
    static const GLuint NORMAL_OFFSET_FLOATS;
    static const GLuint TEXCOORD_OFFSET_FLOATS;
    static const GLuint VERTEX_STRIDE_FLOATS;
    static const GLvoid * POS_OFFSET_BYTES;
    static const GLvoid * COLOR_OFFSET_BYTES;
    static const GLvoid * NORMAL_OFFSET_BYTES;
    static const GLvoid * TEXCOORD_OFFSET_BYTES;
    static const GLsizei VERTEX_STRIDE_BYTES;
 
  private:
      // shader program handle
    GLuint program;

    // handles to shader uniforms
    GLuint  uProjectionMatrix, uCameraTransform, uWorldViewMatrix, uWorldMatrix, uCameraPosition,
        uSamplerNumber, uTexOffsetX, uTexOffsetY, uTexScaleX, uTexScaleY;

    // handles to shader attributes
      GLuint aPosition, aNormal, aTexCoord;
 
    Perception  *perception;
    unsigned short viewportWidth, viewportHeight;
    float aspectRatio;
    ProjectionMode currentProjectionMode, sceneProjectionMode;
 
    float projectionMatrix[16], viewMatrix[16], worldViewMatrix[16], cameraTransform[16];
    Glade::Vector3f backgroundColor;
 
    std::vector<GladeObject*> sceneObjects; // maybe set?

    bool initialized;

  public:
    Renderer(void);
    void onSurfaceCreated();
    void onSurfaceChanged(int width, int height);
    void add(GladeObject *sceneObject);
    void remove(GladeObject *sceneObject);
    void clear(void);
    void onDrawFrame(void);
    void setSceneProjectionMode(ProjectionMode projectionMode);
    int getViewportWidth(void);
    int getViewportHeight(void);
    void setBackgroundColor(float r, float g, float b);
    int percentToPixels(float percent);
    float pixelsToPercent(float pixels);
    void moveZeroToUpperLeftCorner(void);
    float getHalfViewportWidthCoords(void);
    float getHalfViewportHeightCoords(void);
    float getViewportWidthCoords(void);
    float getViewportHeightCoords(void);
    Glade::Vector2f getPointCoords(float screenX, float screenY);
    Glade::Vector3f unprojectPoint(float x, float y, float z);

    Transform *getCamera()
    {
      return perception ? perception->getTransform() : NULL;
    }
    
    Perception  *getPerception() { return perception; }
    void setPerception(Perception *perception) { this->perception = perception; }
    
  private:
    void moveAllObjectsIntoVideoMemory(void);
    void moveIntoVideoMemory(GladeObject &sceneObject);

    void compileShaderProgram(ShaderProgram *program);
    void bindBuffers(Mesh *mesh);
    GLuint loadShader(GLuint type, std::vector<char> &shader_source);
    void moveIntoVideoMemory(std::shared_ptr<Mesh> mesh);
    void moveIntoVideoMemory(std::shared_ptr<Texture> texture);
    void writeUniformsToVideoMemory(Drawable *drawable, ShaderProgram &program);

    int checkGLError();

    void drawAll(std::vector<GladeObject*>::iterator i, std::vector<GladeObject*>::iterator end);
    void removeFromVideoMemory(Drawable &drawable);
    void removeAllObjectsFromVideoMemory(void);
    void draw(GladeObject::DrawablesI di, Transform &transform);
    void switchProjectionMode(ProjectionMode projectionMode, bool force);
    void switchProjectionMode(ProjectionMode projectionMode);
    void getShaderHandles(ShaderProgram &shaderProgram);
  };
}
