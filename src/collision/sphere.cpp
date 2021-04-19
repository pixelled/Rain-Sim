#include <nanogui/nanogui.h>

#include "../mesh.h"
#include "../misc/sphere_drawing.h"
#include "sphere.h"

using namespace nanogui;
using namespace CGL;

void Sphere::collide(Vertex &v) {
    // (Part 3): Handle collisions with spheres.
    Vector3D p = v.position - origin;
    if (p.norm2() < radius2) {
        Vector3D tangent = origin + p.unit() * radius;
        // Raindrop hits sphere here
    }
}

void Sphere::render(GLShader &shader) {
    // We decrease the radius here so flat triangles don't behave strangely
    // and intersect with the sphere when rendered
    m_sphere_mesh.draw_sphere(shader, origin, radius * 0.92);
}
