#ifndef _CLIP_FUNCTIONS_H_
#define _CLIP_FUNCTIONS_H_

#include "csg_classes.h"

#define BVH_OPTIMIZE
#define OPTIMIZE_LOOPS
//#define OPTIMIZE_CLASSIFY_EDGES

#define FACE_GEOMETRY_CHANGED true
#define FACE_GEOMETRY_UNCHANGED false

void reset_clipcpp_timers();
void print_clipcpp_timers();
void reset_testclip_timers();
void print_testclip_timers();

struct clip_results;

template<bool bAccelerate>
void make_result(polyfold& pf4, polyfold& pf5, polyfold& pf, polyfold& pf2, int rule, int rule2, bool overwrite, polyfold& result, clip_results& results, LineHolder& graph);

int do_intersections_and_bisect(polyfold& pf, polyfold& pf2, polyfold& pf4, polyfold& pf5, poly_intersection_info& intersect_info, LineHolder& graph);

//void do_common_topology(polyfold& pf, polyfold& pf2, polyfold& pf4, polyfold& pf5, const std::vector<BVH_intersection_struct>&, LineHolder& graph);
//void do_topology_groups(polyfold& pf4, polyfold& pf2, int rule, int rule2, LineHolder& graph);

void do_self_topology_loops(polyfold& pf, const polyfold& pf0, LineHolder&);
void do_clear_redundant_inner_loops(polyfold& pf, int f_i);
bool is_identical_loop(const polyfold& pf, const poly_loop& loop_a, const polyfold& pf2, const poly_loop& loop_b);

template<bool>
int classify_loop(polyfold& pf, int f_i, int p_i, polyfold& pf2, int rule, int rule2, LineHolder& graph);

//void do_self_intersections(polyfold& pf, const std::vector<BVH_intersection_struct>& );
//void do_initial_topology(polyfold& pf4, polyfold& pf5, const std::vector<BVH_intersection_struct>&, LineHolder& graph);

void clip_poly_no_acceleration(polyfold& pf, polyfold& pf2, int rule, int base_type, clip_results& results, LineHolder& graph);
void clip_poly_accelerated(polyfold& pf, polyfold& pf2, int rule, int base_type, clip_results& results, LineHolder& graph);

template<bool bAccelerate>
int classify_loop(polyfold& pf, int f_i, int p_i, polyfold& pf2, int rule, int rule2, LineHolder& graph)
{
    std::vector<int> tempv = pf.faces[f_i].loops[p_i].vertices;
    tempv.push_back(tempv[0]);

    int e_i;

    std::vector<u16> hits;

    for (int i = 0; i < tempv.size() - 1; i++)
    {
        if(bAccelerate)
            e_i = pf.find_edge_accelerated(pf.vertices[tempv[i]].V, pf.vertices[tempv[i + 1]].V, hits);
        else
            e_i = pf.find_edge(pf.vertices[tempv[i]].V, pf.vertices[tempv[i + 1]].V);

        if (pf.edges[e_i].topo_group == TOP_FRONT || pf.edges[e_i].topo_group == TOP_BEHIND)
            return pf.edges[e_i].topo_group;
    }

    if(bAccelerate)
        e_i = pf.find_edge_accelerated(tempv[0], tempv[1], hits);
    else
        e_i = pf.find_edge(tempv[0], tempv[1]);

    core::vector3df v1;

    LineHolder nograph;

    if (pf.get_facial_point(f_i, e_i, p_i, v1, nograph))
    {
        int RES = pf2.classify_point(v1, nograph);

        if (RES == TOP_UNDEF)
        {
            RES = pf2.classify_point(v1, graph);
            graph.points.push_back(v1);
            std::cout << " classify point returned UNDEF, using default value\n";

            if (pf2.topology == TOP_CONCAVE)
                RES = TOP_BEHIND;
            else if (pf2.topology == TOP_CONVEX)
                RES = TOP_FRONT;
            else
            {
                std::cout << " *error, topology undefined: " << pf2.topology << "\n";
                RES = TOP_UNDEF;
            }
        }

        if (rule == GEO_SUBTRACT)
            RES = !RES;

        return RES;
    }

    std::cout << "*Error classify_loop*\n";
    return TOP_UNDEF;
}

template<bool bAccelerate>
void polyfold::finalize_clipped_poly(const polyfold& pf, LineHolder& graph)
{
    for (int f_i = 0; f_i < this->faces.size(); f_i++)
    {

#ifdef OPTIMIZE_LOOPS
        if (pf.faces[f_i].temp_b == FACE_GEOMETRY_UNCHANGED)
            continue;
#endif

        do_clear_redundant_inner_loops(*this, f_i);

        bool rebuild_loops = false;

        for (int p_i = 0; p_i < this->faces[f_i].loops.size(); p_i++)
        {
            if (this->faces[f_i].loops[p_i].topo_group == LOOP_HOLLOW && this->faces[f_i].loops[p_i].depth == 0)
            {
                this->faces[f_i].loops[p_i].vertices.clear();
                rebuild_loops = true;
            }
        }

        if (rebuild_loops)
        {
            std::vector<poly_loop> new_loops;

            for (int p_i = 0; p_i < this->faces[f_i].loops.size(); p_i++)
            {
                if (faces[f_i].loops[p_i].vertices.size() > 2)
                    new_loops.push_back(faces[f_i].loops[p_i]);
            }

            faces[f_i].loops = new_loops;
        }
    }
    
    if(bAccelerate)
        this->reduce_edges_vertices_accelerated(pf,graph);
    else
        this->reduce_edges_vertices();

    for (int i = 0; i < this->faces.size(); i++)
    {

#ifdef OPTIMIZE_LOOPS
        if (pf.faces[i].temp_b == FACE_GEOMETRY_UNCHANGED)
            continue;
#endif

        for (int p = 0; p < this->faces[i].loops.size(); p++)
        {
            if (this->faces[i].loops[p].vertices.size() > 0)
            {
                    this->set_loop_solid(i, p);
            }
        }
    }

    this->recalc_bbox();
}

template<bool bAccelerate>
void polyfold::rebuild_loops_copy_verts(const polyfold& pf)
{
    std::vector<u16> hits;

    for (int f_i = 0; f_i < faces.size(); f_i++)
    {
        poly_face& f = faces[f_i];
        f.vertices.clear();

        for (poly_loop& loop : faces[f_i].loops)
        {
            for (int& v_i : loop.vertices)
            {
                if (bAccelerate)
                    v_i = pf.find_point_accelerated(this->vertices[v_i].V);
                else
                    v_i = pf.find_point(this->vertices[v_i].V);
                f.addVertex(v_i);
            }
        }

        for (int& e_i : f.edges)
        {
            vector3df v0 = this->getVertex(e_i, 0).V;
            vector3df v1 = this->getVertex(e_i, 1).V;
            if (bAccelerate)
                e_i = pf.find_edge_accelerated(v0, v1, hits);
            else
                e_i = pf.find_edge(v0, v1);
        }
    }
}

template<bool bAccelerate>
void polyfold::rebuild_faces_from_loops(const polyfold& pf)
{
    std::vector<u16> hits;

    for (int f_i = 0; f_i < faces.size(); f_i++)
    {
        poly_face& f = faces[f_i];
        f.vertices.clear();
        f.edges.clear();

        for (poly_loop& loop : faces[f_i].loops)
        {
            for (int i = 0; i < loop.vertices.size(); i++)
            {
                f.addVertex(loop.vertices[i]);
                if (i + 1 < loop.vertices.size())
                {
                    if (bAccelerate)
                        f.edges.push_back(pf.find_edge_accelerated(loop.vertices[i], loop.vertices[i + 1], hits));
                    else
                        f.edges.push_back(find_edge(loop.vertices[i], loop.vertices[i + 1]));
                }
            }

            if (bAccelerate)
                f.edges.push_back(pf.find_edge_accelerated(loop.vertices[0], loop.vertices[loop.vertices.size() - 1], hits));
            else
                f.edges.push_back(find_edge(loop.vertices[0], loop.vertices[loop.vertices.size() - 1]));
        }
    }
}

#endif