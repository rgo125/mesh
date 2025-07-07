#pragma once

#include <vector>
#include <map>
#include <tuple>

#include "meshdata.h"
#include "atomic_operations.h"
#include "operations.h"

#include "Eigen/StdVector"
#include "Eigen/Dense"
#include <list>

EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix2f);
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix3f);
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix3i);

class Mesh
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    void initFromVectors(const std::vector<Eigen::Vector3f> &vertices,
                         const std::vector<Eigen::Vector3i> &faces);

    void loadFromFile(const std::string &filePath);
    void saveToFile(const std::string &filePath);
    void createDataStructure();
    void test_edge_flip();
    void test_edge_collapse();
    void test_edge_split();
    void _subdivide();
    void _denoise();
    void repopulate_obj_file();

private:

    struct TupleHash {
        template <typename T1, typename T2>
        std::size_t operator()(const std::tuple<T1, T2>& tuple) const {
            auto h1 = std::hash<T1>{}(std::get<0>(tuple));
            auto h2 = std::hash<T2>{}(std::get<1>(tuple));
            return h1 ^ (h2 << 1); // Combine hashes
        }
    };

    std::vector<Eigen::Vector3f> _vertices;
    std::vector<Eigen::Vector3i> _faces;
    std::vector<HalfEdge*> _halfedges;
    std::vector<Vertex*> _Vertices;
    //this maps halfedges to their connected verticies, vertex 1 being the back one and vertex 2 being the front one
    std::unordered_map<std::tuple<Vertex*, Vertex*>, std::vector<HalfEdge*>, TupleHash> _vertex_halfedge_map;
    void put_halfedge_in_map(HalfEdge* he1, HalfEdge* he2);
    void validateMesh();
};
