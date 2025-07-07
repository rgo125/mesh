#include "meshdata.h"
#include <iostream>
#include <cassert>

meshdata::meshdata() {

}

void meshdata::validate_halfedge(HalfEdge* h){
    //check for nullptrs first
    assert(h->vertex != nullptr);
    //assert(h->face != nullptr);
    assert(h->next != nullptr);
    assert(h->twin != nullptr);
    //assert(h->edge != nullptr);
    assert(h->vertex->halfedge != nullptr);
    //assert(h->face->halfedge != nullptr);
    //assert(h->edge->halfedge != nullptr);
    //for the operation involving the halfedge's twin we use an if statement to check if it even has a twin
    //with the convention I used, when there is no twin, the halfedge's twin is just set to itself (this makes more sense if you look at my createMesh() function in mesh.cpp)
    //the halfedge must have a face, vertex and edge associated with it, but it doesn't necessarily need a twin since it could be at the edge of the mesh
    if(h->twin != h){
        //make sure the twin does not share the same face
        //assert(h->twin->face != h->face);
        //the halfedge's twin's vertex should not be the same as original halfedges vertex since it's pointing in a different direction
        assert(h->twin->vertex != h->vertex);

        //make sure that the twin of the halfedge's twin has a reference to a twin which is just the original halfedge
        assert(h->twin->twin == h);
        //check that the hafledge's twin's *next references the same vertex as original halfedge
        assert(h->twin->next->vertex == h->vertex);

        //make sure that the halfedge's twin references the same edge
        //assert(h->twin->edge == h->edge);
        //make sure that the edge references either the halfedge or the halfedge's twin
        //assert(h->edge->halfedge == h || h->edge->halfedge == h->twin);
    }

    //the next of the next of the next should just be the half edge we started out with since we've fully wrapped around the triangle
    assert(h == h->next->next->next);
    //make sure that all the nexts share the same face
    //assert(h->face == h->next->face && h->next->face == h->next->next->face);
    //make sure that the vertex of the halfedge points to a halfedge which also points to that vertex
    //note that the vertex doesn't need to point to the original halfedge but it needs to point to a halfedge that points back to said vertex
    assert(h->vertex->halfedge->vertex == h->vertex);

    //here we use a do while loop to make sure that the halfedge can be "reaccessed" by looping through he->twin->next over and over
    //note that if we are at the edge of a mesh this won't be true so we use an if statement to check for this case
    Vertex* v = h->vertex;
    HalfEdge *he = v->halfedge;
    int degree_count = 0;

    do{
        //make sure we're not at edge of mesh
        if(he->twin != he){
            he = he->twin->next;
            degree_count++;
        }
        //if we are at the edge of mesh, just set he to h so that we can leave the while loop and the assertion test passes
        else{
            he = h;
        }
    }while(he != v->halfedge);
    assert(he == v->halfedge);
    assert(v->degree == degree_count);

    //for this next one we want to use a while loop to make sure that the halfedge's vertex references a half edge that we can access by checking
    //all the halfedge.twin.nexts which should eventually get us to the halfedge, the vertex references
    while(he != h->vertex->halfedge){
        //check that we're not at edge of mesh
        if(he->twin != he){
            he = he->twin->next;
        }
        else{
            he = h->vertex->halfedge;
        }
    }
    assert(he == h->vertex->halfedge);


}
