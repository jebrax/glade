#include <glade/physics/IsosurfaceCellPhysicalObject.h>
#include <glade/generation/Grid.h>
#include <glade/GladeObject.h>
#include <glade/render/meshes/Mesh.h>
#include <glade/render/Drawable.h>

#include <ccd/ccd.h>
#include <ccd/quat.h>

static void isosurfaceCellSupportFunction(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *point)
{
  Grid::Cell *cell = (Grid::Cell*) obj;
  GladeObject *entity = cell->getChunkEntity();

  ccd_quat_t q, qx, qy, qz, qinv;
  ccd_vec3_t axisx, axisy, axisz;
  ccdVec3Set(&axisx, 1, 0, 0);
  ccdVec3Set(&axisy, 0, 1, 0);
  ccdVec3Set(&axisz, 0, 0, 1);

  ccdQuatSetAngleAxis(&qx, entity->getTransform()->rotation->x, &axisx);
  ccdQuatSetAngleAxis(&qy, entity->getTransform()->rotation->y, &axisy);
  ccdQuatSetAngleAxis(&qz, entity->getTransform()->rotation->z, &axisz);

  ccdQuatCopy(&q, &qx);
  ccdQuatMul(&q, &qy);
  ccdQuatMul(&q, &qz);

  ccdQuatInvert2(&qinv, &q);

  ccd_vec3_t dir_rotated;
  ccdVec3Set(&dir_rotated, ccdVec3X(dir), ccdVec3Y(dir), ccdVec3Z(dir));
  ccdQuatRotVec(&dir_rotated, &qinv);

  Glade::Vector3f direction(ccdVec3X(&dir_rotated), ccdVec3Y(&dir_rotated), ccdVec3Z(&dir_rotated));
  direction.normalize();

  int startingIndex = cell->startingVertexIndex * 8;
  //log("Starting index: %d, num verts: %d * 8", startingIndex, cell->numVertices);

  std::shared_ptr<Glade::Mesh> mesh = entity->getView()->getMesh();
  Glade::Vector3f vertex(mesh->vertices[startingIndex], mesh->vertices[startingIndex + 1], mesh->vertices[startingIndex + 2]);
  float maxDot = vertex.dot(direction);
  int maxi = startingIndex;
  //log("Box vertex: %f, %f, %f", vertex.x, vertex.y, vertex.z);

  for (int i = startingIndex + 8; i < startingIndex + cell->numVertices * 8; i += 8) {
    Glade::Vector3f vertex(mesh->vertices[i], mesh->vertices[i + 1], mesh->vertices[i + 2]);
    //log("Box vertex: %f, %f, %f", vertex.x, vertex.y, vertex.z);
    float dot = vertex.dot(direction);

    if (dot > maxDot) {
      maxDot = dot;
      maxi = i;
    }
  }

  ccdVec3Set(point, mesh->vertices[maxi + 0], mesh->vertices[maxi + 1], mesh->vertices[maxi + 2]);

  ccdQuatRotVec(point, &q);

  ccd_vec3_t translation;
  ccdVec3Set(&translation, entity->getTransform()->position->x, entity->getTransform()->position->y, entity->getTransform()->position->z);
  ccdVec3Add(point, &translation);
}

void* IsosurfaceCellPhysicalObject::getGjkSupportFunction() {
  return (void*) isosurfaceCellSupportFunction;
}
