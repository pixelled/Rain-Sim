#include "iostream"
#include <nanogui/nanogui.h>

#include "../mesh.h"
#include "../rainSimulator.h"
#include "plane.h"

using namespace std;
using namespace CGL;

#define SURFACE_OFFSET 0.0001

void Plane::collide(Vertex &v) {
    // (Part 3): Handle collisions with planes.
    double dist = dot(v.position - point, normal);
    if (dist <= 0) {
        Vector3D tangent = v.position + normal * (SURFACE_OFFSET - dist);
        // Raindrop hits plane here
    }
}

bool Plane::collide(Raindrop &s, Vector3D &pos) {
    double dist = dot(s.pos - point, normal);
    if (dist <= 0) {
        pos = s.pos - dist;
        return true;
    }

    return false;
}

void Plane::render(GLShader &shader) {
    nanogui::Color color(0.7f, 0.7f, 0.7f, 1.0f);

    Vector3f sPoint(point.x, point.y, point.z);
    Vector3f sNormal(normal.x, normal.y, normal.z);
    Vector3f sParallel(normal.y - normal.z, normal.z - normal.x,
                       normal.x - normal.y);
    Vector2f bottomleft(0.0, 0.0);
    Vector2f topleft(0.0, 1.0);
    Vector2f bottomright(1.0, 0.0);
    Vector2f topright(1.0, 1.0);

    sParallel.normalize();
    Vector3f sCross = sNormal.cross(sParallel);

    MatrixXf positions(3, 4);
    MatrixXf normals(3, 4);
    MatrixXf texcoords(2, 4);

    positions.col(0) << sPoint + 2 * (sCross + sParallel);
    positions.col(1) << sPoint + 2 * (sCross - sParallel);
    positions.col(2) << sPoint + 2 * (-sCross + sParallel);
    positions.col(3) << sPoint + 2 * (-sCross - sParallel);

    normals.col(0) << sNormal;
    normals.col(1) << sNormal;
    normals.col(2) << sNormal;
    normals.col(3) << sNormal;

    texcoords.col(0) << topleft;
    texcoords.col(1) << topright;
    texcoords.col(2) << bottomleft;
    texcoords.col(3) << bottomright;

    if (shader.uniform("u_color", false) != -1) {
        shader.setUniform("u_color", color);
    }
    shader.uploadAttrib("in_position", positions);
    if (shader.attrib("in_normal", false) != -1) {
        shader.uploadAttrib("in_normal", normals);
    }
    shader.uploadAttrib("in_texcoords", texcoords);

    shader.drawArray(GL_TRIANGLE_STRIP, 0, 4);
}
