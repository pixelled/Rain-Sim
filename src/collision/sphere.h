#ifndef COLLISIONOBJECT_SPHERE_H
#define COLLISIONOBJECT_SPHERE_H

#include "../mesh.h"
#include "../misc/sphere_drawing.h"
#include "collisionObject.h"

using namespace CGL;
using namespace std;

struct Sphere : public CollisionObject {
public:
  Sphere(const Vector3D &origin, double radius, bool is_reflected, double friction,
         int num_lat = 40, int num_lon = 40)
      : origin(origin), radius(radius), radius2(radius * radius),
        is_reflected(is_reflected), friction(friction),
        m_sphere_mesh(Misc::SphereMesh(num_lat, num_lon)) {}

  void render(GLShader &shader);
  void collide(Vertex &pm);

  Vector3D origin;
  bool is_reflected;

private:
  double radius;
  double radius2;

  double friction;

  Misc::SphereMesh m_sphere_mesh;
};

#endif /* COLLISIONOBJECT_SPHERE_H */
