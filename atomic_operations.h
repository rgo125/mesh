#ifndef ATOMIC_OPERATIONS_H
#define ATOMIC_OPERATIONS_H

#include "meshdata.h"

class atomic_operations
{
public:
    atomic_operations();
    static void edge_flip(Vertex* v1, Vertex* v2);
    static void edge_collapse(Vertex* v1, Vertex* v2, std::vector<Vertex*>* vertices, std::vector<HalfEdge*>* halfedges);
    static void edge_split(Vertex* v1, Vertex* v2, std::vector<Vertex*>* vertices, std::vector<HalfEdge*>* halfedges);
    static void denoise_point(Vertex* v);

private:
    static HalfEdge* find_shared_edge(Vertex* v1, Vertex* v2);
    static void flip_vertices(HalfEdge* halfedge);
    static void flip_halfedges(HalfEdge* halfedge);
    static void midpoint_vertex(Vertex* v1, Vertex* v2, std::vector<Vertex*>* vertices, std::vector<HalfEdge*>* halfedges, bool collapse);
    static void set_new_vertex(Vertex* old_vertex, Vertex* new_vertex);
    static void collapse_halfedges(HalfEdge* halfedge, std::vector<HalfEdge*>* halfedges);
    static void remove_vertex(Vertex* vertex, std::vector<Vertex*>* vertices);
    static void remove_halfedge(HalfEdge* halfedge, std::vector<HalfEdge*>* halfedges);
    static void add_new_halfedges(HalfEdge* halfedge, Vertex* vertex, std::vector<HalfEdge*>* halfedges);
    static Eigen::Vector3f find_vert_normal(Vertex* v);
    static std::vector<Vertex*> get_neighbors(Vertex* v);
};

#endif // ATOMIC_OPERATIONS_H
