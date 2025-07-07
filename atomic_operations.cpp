#include "atomic_operations.h"
#include <iostream>

atomic_operations::atomic_operations() {}

/*
 * THESE FIRST FEW METHODS HAVE TO DO WITH EDGE FLIP
 *
 *
 *
 */
void atomic_operations::edge_flip(Vertex* v1, Vertex* v2){
    //first we get the halfedge corresponding to the edge between the two vertices
    //this halfedge will be pointing to v1
    HalfEdge* halfedge = find_shared_edge(v1, v2);
    //reset vertex data for halfedge and twin that we're flipping
    flip_vertices(halfedge);
    flip_vertices(halfedge->twin);

    //next we need to manipulate all the local halfedges
    flip_halfedges(halfedge);
}

HalfEdge* atomic_operations::find_shared_edge(Vertex* v1, Vertex* v2){
    //initialize hedge as v1's halfedge
    HalfEdge* hedge = v1->halfedge;
    //keep looping through the halfedge.twin.next since this looks at all the hafledges pointing to v1
    //we're looking for the one whose twin points to v2
    while(hedge->twin->vertex != v2){
        hedge = hedge->twin->next;
    }
    return hedge;
}

void atomic_operations::flip_vertices(HalfEdge* halfedge){
    //if the vertex points to the halfedge we want it to point to another halfedge
    if(halfedge->vertex->halfedge == halfedge){
        //set it to twin->next since that's the next hafledge that points to the vertex
        halfedge->vertex->halfedge = halfedge->twin->next;
    }
    halfedge->vertex->degree--;
    //then set the halfedge's vertex to twin->next->next->vertex
    halfedge->vertex = halfedge->twin->next->next->vertex;
    halfedge->twin->next->next->vertex->degree++;
}

void atomic_operations::flip_halfedges(HalfEdge* halfedge){
    //we need to store the next and next next for next and next->next for both the halfedge and it's twin
    HalfEdge* next = halfedge->next;
    HalfEdge* next_next = halfedge->next->next;

    HalfEdge* twin_next = halfedge->twin->next;
    HalfEdge* twin_next_next = halfedge->twin->next->next;

    //reset all the halfedge references
    halfedge->next = next_next;
    next->next = halfedge->twin;
    next_next->next = twin_next;

    //as well as twin references
    halfedge->twin->next = twin_next_next;
    twin_next->next = halfedge;
    twin_next_next->next = next;
}


/*
 * NEXT FEW METHODS HAVE TO DO WITH EDGE COLLAPSE
 *
 *
 *
 */
void atomic_operations::edge_collapse(Vertex* v1, Vertex* v2,
                                      std::vector<Vertex*>* vertices,
                                      std::vector<HalfEdge*>* halfedges){
    //we need to look at all the halfedge's we're getting rid of
    HalfEdge* halfedge = find_shared_edge(v1, v2);
    //then get rid of references to v1 and v2 and create a new vertex to replace them
    midpoint_vertex(v1, v2, vertices, halfedges, true);
    //here we should remove v1 and v2 from the vector of vertices
    remove_vertex(v1, vertices);
    remove_vertex(v2, vertices);
    //collapse face connected to halfedge
    //do the same with it's twin
    HalfEdge* twin = halfedge->twin;
    collapse_halfedges(halfedge, halfedges);
    collapse_halfedges(twin, halfedges);
}

void atomic_operations::midpoint_vertex(Vertex* v1, Vertex*v2,
                                        std::vector<Vertex*>* vertices,
                                        std::vector<HalfEdge*>* halfedges,
                                        bool collapse){
    Vertex* midpoint = new Vertex;
    //set the position of the midpoint to be in between v1 and v2
    midpoint->position = v1->position + ((v2->position - v1->position)/2.f);
    //if we're collapsing, set all the halfedges connected to v1 and v2 to reference this new vertex
    if(collapse){
        set_new_vertex(v1, midpoint);
        set_new_vertex(v2, midpoint);
    }
    else{
        HalfEdge* he = find_shared_edge(v1, v2);
        add_new_halfedges(he, midpoint, halfedges);
    }
    vertices->push_back(midpoint);
}

void atomic_operations::set_new_vertex(Vertex* old_vertex, Vertex* new_vertex){
    HalfEdge* hedge = old_vertex->halfedge;
    //loop through all the halfedges connected to the old_vertex
    do{
        //set hedge vertex to the new one
        hedge->vertex = new_vertex;
        hedge = hedge->twin->next;
    } while(hedge != old_vertex->halfedge);//once we've done a full rotation, end the loop
}

void atomic_operations::collapse_halfedges(HalfEdge* halfedge, std::vector<HalfEdge*>* halfedges){
    HalfEdge* next_twin = halfedge->next->twin;
    HalfEdge* next_next_twin = halfedge->next->next->twin;

    //reset twin to close the gap
    next_twin->twin = next_next_twin;
    next_next_twin->twin = next_twin;

    //make sure that the vertex doesn't point to a halfedge that we are deleting
    next_twin->vertex->halfedge = next_twin;
    //also we set the new vertex we created earlier to be pointing to one of the ones we didn't delete
    next_next_twin->vertex->halfedge = next_next_twin;

    //here we should delete the useless halfedges from the vector of halfedges
    remove_halfedge(halfedge->next->next, halfedges);
    remove_halfedge(halfedge->next, halfedges);
    remove_halfedge(halfedge, halfedges);
}

void atomic_operations::remove_vertex(Vertex* vertex, std::vector<Vertex*>* vertices){
    vertices->erase(std::remove(vertices->begin(), vertices->end(), vertex), vertices->end());
}

void atomic_operations::remove_halfedge(HalfEdge* halfedge, std::vector<HalfEdge*>* halfedges){
    halfedges->erase(std::remove(halfedges->begin(), halfedges->end(), halfedge), halfedges->end());
}

/*
 *NEXT FEW METHODS DEAL WITH EDGE SPLIT
 *
 *
 *
*/

void atomic_operations::edge_split(Vertex* v1, Vertex* v2, std::vector<Vertex*>* vertices, std::vector<HalfEdge*>* halfedges){
    midpoint_vertex(v1, v2, vertices, halfedges, false);
}

void atomic_operations::add_new_halfedges(HalfEdge* halfedge, Vertex* new_vertex, std::vector<HalfEdge*>* halfedges){
    //we need to store the next and next next for next and next->next for both the halfedge and it's twin
    HalfEdge* next = halfedge->next;
    HalfEdge* next_next = halfedge->next->next;

    HalfEdge* he_twin = halfedge->twin;
    HalfEdge* twin_next = halfedge->twin->next;
    HalfEdge* twin_next_next = halfedge->twin->next->next;

    //top right
    halfedge->vertex->halfedge = twin_next;
    halfedge->vertex = new_vertex;
    new_vertex->halfedge = halfedge;
    new_vertex->degree++;

    HalfEdge* new_next_next = new HalfEdge;
    next->next = new_next_next;
    new_next_next->vertex = next_next->vertex;
    next_next->vertex->degree++;
    new_next_next->next = halfedge;
    new_next_next->flip_me = true;
    halfedges->push_back(new_next_next);

    //top left
    HalfEdge* new_he = new HalfEdge;
    HalfEdge* new_next = new HalfEdge;

    next_next->next = new_he;
    new_he->next = new_next;
    new_he->vertex = twin_next->vertex;
    halfedges->push_back(new_he);

    new_next->next = next_next;
    new_next->vertex = new_vertex;
    new_vertex->degree++;
    new_next->twin = new_next_next;
    new_next_next->twin = new_next;
    new_next->flip_me = true;
    halfedges->push_back(new_next);

    //bottom left
    HalfEdge* new_twin = new HalfEdge;
    HalfEdge* new_twin_next_next = new HalfEdge;

    new_twin->twin = new_he;
    new_he->twin = new_twin;
    new_twin->vertex = new_vertex;
    new_vertex->degree++;
    new_twin->next = twin_next;
    halfedges->push_back(new_twin);

    twin_next->next = new_twin_next_next;
    new_twin_next_next->next = new_twin;
    new_twin_next_next->vertex = twin_next_next->vertex;
    twin_next_next->vertex->degree++;
    new_twin_next_next->flip_me = true;
    halfedges->push_back(new_twin_next_next);

    //bottom right
    HalfEdge* new_twin_next = new HalfEdge;
    he_twin->next = new_twin_next;
    new_twin_next->next = twin_next_next;
    new_twin_next->vertex = new_vertex;
    new_vertex->degree++;
    new_twin_next->flip_me = true;
    new_twin_next->twin = new_twin_next_next;
    new_twin_next_next->twin = new_twin_next;
    halfedges->push_back(new_twin_next);

}



void atomic_operations::denoise_point(Vertex* v){
    std::vector<Vertex*> neighbors = get_neighbors(v);

    Eigen::Vector3f norm = find_vert_normal(v);
    float sum = 0.f;
    float normalizer = 0.f;


    for(int i = 0; i < neighbors.size(); i++){
        float t = (v->position - neighbors[i]->position).norm();
        float h = norm.dot(v->position - neighbors[i]->position);
        float w_c = exp(-(pow(t, 2.f))/(2.f * pow(0.2f, 2.f)));
        float w_s = exp(-(pow(h, 2.f))/(2.f * pow(0.2f, 2.f)));
        sum += w_c * w_s * h;
        normalizer += w_c * w_s;
    }
    v->position = v->position - (norm * (sum/normalizer));
}
Eigen::Vector3f atomic_operations::find_vert_normal(Vertex* v){
    //intitalize he
    HalfEdge* he = v->halfedge;
    //initialize normal sum as a 0 vector
    Eigen::Vector3f normals_sum = {0.f, 0.f, 0.f};
    //loop through all the neighbors
    do{
        he = he->twin->next;
        Vertex* v1 = he->next->vertex;
        Vertex* v2 = he->next->next->vertex;
        Eigen::Vector3f face_norm = ((v1->position - v->position).normalized()).cross((v2->position - v->position).normalized()).normalized();
        normals_sum += face_norm;

    }while(he != v->halfedge);

    return normals_sum.normalized();
}

std::vector<Vertex*> atomic_operations::get_neighbors(Vertex* v){
    std::vector<Vertex*> vertices;
    HalfEdge* he = v->halfedge;
    do{
        he = he->twin->next;
        vertices.push_back(he->twin->vertex);

    }while(he != v->halfedge);
    return vertices;
}

