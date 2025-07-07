#include "operations.h"
#include <iostream>
#include <cmath>

operations::operations() {}

void operations::subdivide(std::vector<Vertex*>* vertices, std::vector<HalfEdge*>* halfedges){
    //first we split every halfedge in the list of halfedge
    //but we should also store size of the list of hafledges, since the splits will add new halfedges to the list but we
    //dont do anything to those new ones yet
    int initial_size = halfedges->size();
    int initial_vertices_size = vertices->size();
    //iterate through list
    for(int i = 0; i < halfedges->size(); i++){
        //dereference the iterator
        auto he = (*halfedges)[i];
        //make sure we haven't visited the halfedge yet
        if(!(he->vertex->new_one) && !(he->twin->vertex->new_one)){
            //split the edge using the halfedge we're looking at
            atomic_operations::edge_split(he->vertex, he->twin->vertex, vertices, halfedges);
            //then set the bool new_one to be true for the last vertex in vertices since we just created this
            vertices->back()->new_one = true;
        }
    }


    for(int i = initial_size; i < halfedges->size(); i++){
        HalfEdge* he = (*halfedges)[i];
         //make sure we haven't visited this halfedge yet
        if(he->flip_me){
             Vertex* v1 = he->vertex;
             Vertex* v2 = he->twin->vertex;
             //check if the halfedge connect a new vertex and an old one
             if((v1->new_one && !v2->new_one) || (!v1->new_one && v2->new_one)){
                 atomic_operations::edge_flip(v1, v2);
             }
             he->flip_me = false;
             he->twin->flip_me = false;
         }
         //he->visited = false;
     }
    move_new_vertices(vertices, initial_vertices_size);
    move_old_vertices(vertices, initial_vertices_size);
     //before we end the method we want to make sure that all the verticex->new_one is set to false
    for(Vertex* v: *vertices){
        v->new_one = false;
    }
}

void operations::move_new_vertices(std::vector<Vertex*>* vertices, int start){
    for(int i = start; i < vertices->size(); i++){
        Vertex* vert = (*vertices)[i];
        //find the vertex neighbors (that aren't new vertices)
        //first find a connecting halfedge that also connects to an old vertex
        std::vector<Vertex*> neighbors = find_new_neighbors(vert);
        vert->position = (neighbors[0]->position * 3.f/8.f) + (neighbors[1]->position * 1.f/8.f) + (neighbors[2]->position * 3.f/8.f) + (neighbors[3]->position * 1.f/8.f);
    }
}

void operations::move_old_vertices(std::vector<Vertex*>* vertices, int end){
    for(int i = 0; i < end; i++){
        Vertex* vert = (*vertices)[i];
        std::vector<Vertex*> neighbors = find_old_neighbors(vert);
        float u = 0;
        if(vert->degree == 3){
            u = 3.f/16.f;
        }
        else{
            u = (1.f/(1.f * vert->degree)) * ((5.f/8.f) - pow((3.f/8.f) + ((1.f/4.f) * (cos((2.f * M_PI)/(1.f * vert->degree)))), 2.f));
        }

        Eigen::Vector3f pos = {0.f,0.f,0.f};
        for(Vertex* v: neighbors){
            pos += v->position;
        }
        pos = pos * u;
        pos = (1.f - (vert->degree * u)) * vert->position + pos;
        vert->position = pos;
    }
}

std::vector<Vertex*> operations::find_new_neighbors(Vertex* vert){
    HalfEdge* he = vert->halfedge;

    do{
        //loop around vertices halfedges
        he = he->twin->next;
    }while(he->twin->vertex->new_one);//once we get to an old one the while loop terminates

    //*3/8
    Vertex* v1 = he->twin->vertex;
    //*1.8
    Vertex* v2 = he->next->next->twin->next->twin->next->next->vertex;

    //repeat for lower traingle
    do{
        //loop around vertices halfedges
        he = he->twin->next;
    }while(he->twin->vertex->new_one);//once we get to an old one the while loop terminates
    //*3/8
    Vertex* v3 = he->twin->vertex;
    //*1.8
    Vertex* v4 = he->next->next->twin->next->twin->next->next->vertex;

    return{v1, v2, v3, v4};
}

std::vector<Vertex*> operations::find_old_neighbors(Vertex* vert){
    HalfEdge* he = vert->halfedge;
    std::vector<Vertex*> vertices;
    do{
        he = he->twin->next;
        Vertex* v = he->next->twin->next->next->twin->next->next->vertex;
        vertices.push_back(v);
    }while (he != vert->halfedge);

    return vertices;
}

void operations::bilaterial_denoise(std::vector<Vertex*>* vertices){
    for(int i = 0; i < vertices->size(); i++){
        Vertex* vert = (*vertices)[i];
        atomic_operations::denoise_point(vert);
    }
}

