#ifndef MESHDATA_H
#define MESHDATA_H
#include "Eigen/StdVector"
#include "Eigen/Dense"

struct HalfEdge;
struct Vertex;
struct Face;
struct Edge;

struct HalfEdge{
    Face *face;
    Vertex *vertex;
    HalfEdge *next;
    Edge *edge;
    HalfEdge *twin;
    HalfEdge *old_twin;
    bool flip_me = false;
    bool visited = false;
};

struct Vertex{
    HalfEdge *halfedge;
    Eigen::Vector3f position;
    int index = 0;
    bool new_one = false;
    int degree = 0;
};

struct Face{
    HalfEdge *halfedge;
};

struct Edge{
    HalfEdge *halfedge;
};

class meshdata
{
public:
    meshdata();
    static void validate_halfedge(HalfEdge *halfedge);
};

#endif // MESHDATA_H
