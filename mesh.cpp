#include "mesh.h"

#include <iostream>
#include <fstream>

#include <QFileInfo>
#include <QString>

#define TINYOBJLOADER_IMPLEMENTATION
#include "util/tiny_obj_loader.h"

using namespace Eigen;
using namespace std;

void Mesh::initFromVectors(const vector<Vector3f> &vertices,
                           const vector<Vector3i> &faces)
{
    // Copy vertices and faces into internal vector
    _vertices = vertices;
    _faces    = faces;
}

void Mesh::loadFromFile(const string &filePath)
{
    tinyobj::attrib_t attrib;
    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> materials;

    QFileInfo info(QString(filePath.c_str()));
    string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
                                info.absoluteFilePath().toStdString().c_str(), (info.absolutePath().toStdString() + "/").c_str(), true);
    if (!err.empty()) {
        cerr << err << endl;
    }

    if (!ret) {
        cerr << "Failed to load/parse .obj file" << endl;
        return;
    }

    for (size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            unsigned int fv = shapes[s].mesh.num_face_vertices[f];

            Vector3i face;
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                face[v] = idx.vertex_index;
            }
            _faces.push_back(face);

            index_offset += fv;
        }
    }
    for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
        _vertices.emplace_back(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]);
    }
    cout << "Loaded " << _faces.size() << " faces and " << _vertices.size() << " vertices" << endl;
}

void Mesh::saveToFile(const string &filePath)
{
    ofstream outfile;
    outfile.open(filePath);

    // Write vertices
    for (size_t i = 0; i < _vertices.size(); i++)
    {
        const Vector3f &v = _vertices[i];
        outfile << "v " << v[0] << " " << v[1] << " " << v[2] << endl;
    }

    // Write faces
    for (size_t i = 0; i < _faces.size(); i++)
    {
        const Vector3i &f = _faces[i];
        outfile << "f " << (f[0]+1) << " " << (f[1]+1) << " " << (f[2]+1) << endl;
    }

    outfile.close();
}

void Mesh::put_halfedge_in_map(HalfEdge* he1, HalfEdge* he2){
    //back vertex
    Vertex* vertex_1 = he1->vertex;
    //front vertex
    Vertex* vertex_2 = he2->vertex;
    tuple<Vertex*, Vertex*> vert_tupl;
    //check if any halfedges along the same vertices are in the hashmap
    if(_vertex_halfedge_map.contains({vertex_2, vertex_1})){
        vert_tupl = {vertex_2, vertex_1};
    }
    else{
        vert_tupl = {vertex_1, vertex_2};
    }
    //set first halfedge's twin to the last one in the list of halfedges associated with vertices
    //if the back is not null then set our hafledge's edge equal to it
    if(_vertex_halfedge_map.contains(vert_tupl)){
        he1->twin = _vertex_halfedge_map[vert_tupl].back();
        he1->edge = _vertex_halfedge_map[vert_tupl].back()->edge;
    }

    //if it is null, then we want to create a new edge
    else{
        Edge* edge = new Edge();
        he1->edge = edge;
        edge->halfedge = he1;
    }
    //then put the first halfedge into the hashmap
    _vertex_halfedge_map[vert_tupl].push_back(he1);
    //then set the first halfedge in that list's twin to be the previous halfedge
    //if there are only two halfedges associated with the two vertices, then they will just be set to eachother's twins
    //however, if there's a thrid one (meaning it's non-manifold), the third one's twin will be set to the second one and the first one's twin will be set
    //to the third one, which the assertion test will pick up on and raise an error if this is the case
    _vertex_halfedge_map[vert_tupl].front()->twin = he1;
    //it's important to note that if the prev_halfedge is the first halfedge we mapped to this key, then we just set it's twin to be itself
    //we will just use this as the case where we are at the edge of a mesh and there is no twin

}

 void Mesh::createDataStructure(){
    //first just populate the list of our vertex structs using the vertices from tinyobj
    foreach(Vector3f vertex, _vertices){
        Vertex* Vert = new Vertex;
        Vert->position = vertex;
        _Vertices.push_back(Vert);
    }

    for(auto& face : _faces){
        //create a face struct for each face
        Face* face_struct = new Face;
        //make 3 halfedges
        //pointer to the halfedge we previously looked at, will start out as null but then get set within the for loop
        HalfEdge* prev_halfedge = nullptr;
        //also want a pointer to the first halfedge, that way we can set the final halfedge's next to the first one
        HalfEdge* first_halfedge;
        for(int i = 0; i < 3; i++){
            //create a halfedge struct which the face will reference and it will reference the face and vertex[i]
            HalfEdge* halfedge_struct = new HalfEdge;

            halfedge_struct->face = face_struct; //face reference
            halfedge_struct->vertex = _Vertices[face[i]]; //vertex reference

            _Vertices[face[i]]->halfedge = halfedge_struct;//set vertex's halfedge to halfedge_struct
            _Vertices[face[i]]->degree++;

            //put into the list of halfedges
            _halfedges.push_back(halfedge_struct);
            //if were not looking at the first halfedge (meaning that the previous halfedge is not a nullptr),
            //then we should set the previous halfedge's next to the current halfedge
            if(prev_halfedge != nullptr){
                prev_halfedge->next = halfedge_struct;

                //put the previous halfedge into the hashmap
                //we cant put the current one in yet because we haven't processed the vertex in front of it yet
                put_halfedge_in_map(prev_halfedge, _halfedges.back());
            }
            //if this is first halfedge, set first_halfedge pointer
            if(i == 0){
                first_halfedge = _halfedges.back();
            }
            //if we're looking at the final halfedge, set its next to the first halfedge
            if(i == 2){
                _halfedges.back()->next = first_halfedge;
                //now put the last halfedge into the hashmap
                //in this case the vertex in front of it is the vertex that the first halfedge references
                put_halfedge_in_map(_halfedges.back(), first_halfedge);
            }
            prev_halfedge = _halfedges.back();
        }
        //now just set the face_struct's halfedge to be the first halfedge
        face_struct->halfedge = first_halfedge;
    }
    validateMesh();
}

void Mesh::validateMesh(){
    foreach(HalfEdge* halfedge, _halfedges){
        meshdata::validate_halfedge(halfedge);
    }
}

void Mesh::test_edge_flip(){
    HalfEdge* he = _halfedges.back();
    atomic_operations::edge_flip(he->vertex, he->twin->vertex);
    validateMesh();
}

void Mesh::test_edge_collapse(){
    HalfEdge* he = _halfedges.back();
    atomic_operations::edge_collapse(he->vertex, he->twin->vertex, &_Vertices, &_halfedges);
    validateMesh();
}

void Mesh::test_edge_split(){
    HalfEdge* he = _halfedges.back();
    atomic_operations::edge_split(he->vertex, he->twin->vertex, &_Vertices, &_halfedges);
    validateMesh();
}

void Mesh::_subdivide(){
    operations::subdivide(&_Vertices, &_halfedges);
    validateMesh();
}

void Mesh::_denoise(){
    operations::bilaterial_denoise(&_Vertices);
}

void Mesh::repopulate_obj_file(){
    //clear the vertices and faces
    _vertices.clear();
    _faces.clear();
    //repopulate it with the positions from _Vertices
    for(int i = 0; i < _Vertices.size(); i++){
        //set the vertex position as a Vector3f
        _vertices.push_back(_Vertices[i]->position);
        //and then also set the index in the Vertex struct to make it easier to acess when making faces
        _Vertices[i]->index = i;
    }

    //now we need to repopulate faces
    //so lets treat the list of halfedges as a queue
    //we go through each one and create a Vector3i representing a face
    //use the corresponding vertices indices to populate the Vector3i

    for(auto& he_pointer : _halfedges){
        //dereference the pointer
        HalfEdge he = *he_pointer;
        //make sure we haven't visited the halfedge yet
         if(!he.visited){
            //create a Vector3i representing the face
            Vector3i face;
            face.x() = he.vertex->index;
            he.visited = true;

            face.y() = he.next->vertex->index;
            he.next->visited = true;

            face.z() = he.next->next->vertex->index;
            he.next->next->visited = true;

            _faces.push_back(face);
        }
    }
    std::cout<< "faces size: " << _faces.size() << " vertices size: " << _vertices.size() << std::endl;
}
