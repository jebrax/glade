#pragma once

#include <glade/render/meshes/Mesh.h>
#include <BulletCollision/CollisionShapes/btStridingMeshInterface.h>

#include <memory>

namespace Glade {
  class Mesh;
}

class BulletTriangleMeshDecorator: public btStridingMeshInterface
{
public:
  BulletTriangleMeshDecorator(std::shared_ptr<Glade::Mesh> glade_mesh);

  virtual void getLockedVertexIndexBase(unsigned char **vertexbase, int& numverts,PHY_ScalarType& type, int& stride,unsigned char **indexbase,int & indexstride,int& numfaces,PHY_ScalarType& indicestype,int subpart = 0) override;
  virtual void getLockedReadOnlyVertexIndexBase(const unsigned char **vertexbase, int& numverts,PHY_ScalarType& type, int& stride,const unsigned char **indexbase,int & indexstride,int& numfaces,PHY_ScalarType& indicestype,int subpart = 0) const override;
  virtual void unLockVertexBase(int subpart) override;
  virtual void unLockReadOnlyVertexBase(int subpart) const override;
  virtual int getNumSubParts() const override;
  virtual void preallocateVertices(int numverts) override;
  virtual void preallocateIndices(int numindices) override;

private:
  std::shared_ptr<Glade::Mesh> mesh;
};

