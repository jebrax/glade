#include <glade/physics/BulletTriangleMeshDecorator.h>
#include <glade/render/meshes/Mesh.h>
#include <glade/render/GladeRenderer.h>

BulletTriangleMeshDecorator::BulletTriangleMeshDecorator(std::shared_ptr<Glade::Mesh> glade_mesh):
  btStridingMeshInterface(),
  mesh(glade_mesh) {}

void BulletTriangleMeshDecorator::getLockedVertexIndexBase(unsigned char **vertexbase, int& numverts, PHY_ScalarType& type, int& stride, unsigned char **indexbase, int & indexstride, int& numfaces,PHY_ScalarType& indicestype, int subpart)
{
  assert(false && "Non-const access to the mesh is prohibited");
  *vertexbase = (unsigned char*) &mesh->vertices[0];
  numverts = mesh->getVertexBufferSize();
  type = PHY_FLOAT;
  stride = Glade::Renderer::VERTEX_STRIDE_BYTES;

  *indexbase = (unsigned char*) &mesh->indices[0];
  indexstride = 3 * sizeof(unsigned short);
  numfaces = mesh->getIndexBufferSize() / 3;
  indicestype = PHY_SHORT;
}

void BulletTriangleMeshDecorator::getLockedReadOnlyVertexIndexBase(const unsigned char **vertexbase, int& numverts,PHY_ScalarType& type, int& stride, const unsigned char **indexbase, int& indexstride, int& numfaces, PHY_ScalarType& indicestype, int subpart) const
{
  *vertexbase = (const unsigned char*) &mesh->vertices[0];
  numverts = mesh->getVertexBufferSize();
  type = PHY_FLOAT;
  stride = Glade::Renderer::VERTEX_STRIDE_BYTES;

  *indexbase = (const unsigned char*) &mesh->indices[0];
  indexstride = 3 * sizeof(unsigned short);
  numfaces = mesh->getIndexBufferSize() / 3;
  indicestype = PHY_SHORT;
}

void BulletTriangleMeshDecorator::unLockVertexBase(int subpart)
{
  // TODO
}

void BulletTriangleMeshDecorator::unLockReadOnlyVertexBase(int subpart) const
{
  // TODO
}

int BulletTriangleMeshDecorator::getNumSubParts() const
{
  return 1;
}

void BulletTriangleMeshDecorator::preallocateVertices(int numverts)
{
  log("Warning: BulletTriangleMeshDecorator::preallocateVertices() not implemented.");
}

void BulletTriangleMeshDecorator::preallocateIndices(int numindices)
{
  log("Warning: BulletTriangleMeshDecorator::preallocateIndices() not implemented.");
}

