#ifndef CLOTH_MESH_H
#define CLOTH_MESH_H

#include <vector>

#include "CGL/CGL.h"
#include "vertex.h"

using namespace CGL;
using namespace std;

class Triangle {
public:
  Triangle(Vertex *v1, Vertex *v2, Vertex *v3, Vector3D uv1, Vector3D uv2, Vector3D uv3)
      : v1(v1), v2(v2), v3(v3), uv1(uv1), uv2(uv2), uv3(uv3) {}

  // Static references to constituent mesh objects
  Vertex *v1;
  Vertex *v2;
  Vertex *v3;
  
  // UV values for each of the points.
  // Uses Vector3D for convenience. This means that the z dimension
  // is not used, and xy corresponds to uv.
  Vector3D uv1;
  Vector3D uv2;
  Vector3D uv3;

  Halfedge *halfedge;
}; // struct Triangle

class Edge {
public:
  Halfedge *halfedge;
}; // struct Edge

class Halfedge {
public:
  Edge *edge;
  Halfedge *next;
  Halfedge *twin;
  Triangle *triangle;
  Vertex *pm;
}; // struct Halfedge

class Mesh {
public:
  ~Mesh() {}

  vector<Triangle *> triangles;
}; // struct Mesh

#endif // CLOTH_MESH_H
