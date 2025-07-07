#ifndef OPERATIONS_H
#define OPERATIONS_H
#include "atomic_operations.h"
#include "Eigen/StdVector"
#include "Eigen/Dense"

class operations
{
public:
    operations();
    static void subdivide(std::vector<Vertex*>* vertices, std::vector<HalfEdge*>* halfedges);
    static void move_vertices(std::vector<Vertex*>* vertices, int end);
    static void move_old_vertices(std::vector<Vertex*>* vertices, int end);
    static void move_new_vertices(std::vector<Vertex*>* vertices, int start);
    static std::vector<Vertex*> find_old_neighbors(Vertex* vert);
    static std::vector<Vertex*> find_new_neighbors(Vertex* vert);
    static void bilaterial_denoise(std::vector<Vertex*>* vertices);
};

#endif // OPERATIONS_H
