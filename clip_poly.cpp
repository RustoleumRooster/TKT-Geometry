
#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"
#include "edit_env.h"
#include "tolerances.h"
#include <chrono>
#include "clip_functions3.h"    //TODO, dependency 

using namespace irr;

#define TIME_HEADER() auto startTime = std::chrono::high_resolution_clock::now();\
    auto timeZero = startTime;\
    auto currentTime = std::chrono::high_resolution_clock::now();\
    float time;
#define START_TIMER() startTime = std::chrono::high_resolution_clock::now(); \

#define PRINT_TIMER(text) currentTime = std::chrono::high_resolution_clock::now(); \
    time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count(); \
    std::cout << "---------time (" <<#text<< "): " << time << "\n";\

void combine_polyfolds(const std::vector<polyfold*>& polies, polyfold& res)
{
    /*
    if (polies.size() == 0)
        return;
    else if (polies.size() == 1)
    {
        res = *polies[0];
        return;
    }*/

    int n_vertices = 0;

    for (const polyfold* pf : polies)
        n_vertices += pf->vertices.size();

    if (n_vertices > 120)
    {
        combine_polyfolds_accelerated(polies, res);
    }
    else
    {
        combine_polyfolds_linear(polies, res);
    }

}

void polyfold::deduplicate_edges_vertices_accelerated()
{
    vertices_BVH.invalidate();
    vertices_BVH.build(this->vertices.data(), this->vertices.size());
    std::vector<poly_vert> new_verts;

    for (int i = 0; i < vertices_BVH.node_count; i++)
    {
        BVH_node& node = vertices_BVH.nodes[i];
        if (node.isLeafNode())
        {
            u16 v_i = vertices_BVH.indices[node.first_prim];
            new_verts.push_back(vertices[v_i].V);
            vertices_BVH.indices[node.first_prim] = new_verts.size() - 1;
            node.n_prims = 1;
           
        }
    }

    edges_BVH.invalidate();
    edges_BVH.build(vertices.data(), this->edges.data(), this->edges.size());
    std::vector<poly_edge> new_edges;

    for (int i = 0; i < edges_BVH.node_count; i++)
    {
        BVH_node& node = edges_BVH.nodes[i];
        if (node.isLeafNode())
        {
            u16 e_i = edges_BVH.indices[node.first_prim];
            vector3df v0 = getVertex(e_i, 0).V;
            vector3df v1 = getVertex(e_i, 1).V;

            int v_0 = vertices_BVH.find_vertex_nocheck(v0);
            int v_1 = vertices_BVH.find_vertex_nocheck(v1);

            new_edges.push_back(poly_edge(v_0, v_1));
            edges_BVH.indices[node.first_prim] = new_edges.size() - 1;
            node.n_prims = 1;
        }
    }

    for (int f_i = 0; f_i < faces.size(); f_i++)
    {
        faces[f_i].edges.clear();
        faces[f_i].vertices.clear();

        for (int p_i = 0; p_i < faces[f_i].loops.size(); p_i++)
        {
            //if (faces[f_i].loops[p_i].vertices.size() == 0)
            //    continue;

            for (int i = 0; i < faces[f_i].loops[p_i].vertices.size(); i++)
            {
                int v_i = faces[f_i].loops[p_i].vertices[i];
                int new_v = vertices_BVH.find_vertex_nocheck(vertices[v_i].V);

                faces[f_i].loops[p_i].vertices[i] = new_v;
                faces[f_i].addVertex(new_v);
            }

            for (int i = 0; i < faces[f_i].loops[p_i].vertices.size(); i++)
            {
                if (i + 1 < faces[f_i].loops[p_i].vertices.size())
                {
                    int v_i = faces[f_i].loops[p_i].vertices[i];
                    int v_i1 = faces[f_i].loops[p_i].vertices[i + 1];
                    aabb_struct aabb;
                    new_verts[v_i].grow(&aabb);
                    new_verts[v_i1].grow(&aabb);
                    std::vector<u16> hits;
                    edges_BVH.intersect(aabb, hits);
                    for (u16 e_i : hits)
                    {
                        if ((new_edges[e_i].v0 == v_i && new_edges[e_i].v1 == v_i1) || (new_edges[e_i].v1 == v_i && new_edges[e_i].v0 == v_i1) &&
                            (new_edges[e_i].topo_group != -1))
                        {
                            faces[f_i].edges.push_back(e_i);
                        }
                    }
                }
            }

            aabb_struct aabb;
            int v_i0 = faces[f_i].loops[p_i].vertices[0];
            int v_i1 = faces[f_i].loops[p_i].vertices[faces[f_i].loops[p_i].vertices.size() - 1];
            new_verts[v_i0].grow(&aabb);
            new_verts[v_i1].grow(&aabb);
            std::vector<u16> hits;
            edges_BVH.intersect(aabb, hits);
            for (u16 e_i : hits)
            {
                if ((new_edges[e_i].v0 == v_i0 && new_edges[e_i].v1 == v_i1) || (new_edges[e_i].v1 == v_i0 && new_edges[e_i].v0 == v_i1))
                {
                    faces[f_i].edges.push_back(e_i);
                }
            }
            
        }
    }

    vertices = new_verts;
    edges = new_edges;
}

void combine_polyfolds_accelerated(const std::vector<polyfold*>& polies, polyfold& res)
{
    //TIME_HEADER()
    //START_TIMER()

    int nVertices = 0;
    int nEdges = 0;
    int n_surface_groups = 0;

    for (int i = 0; i < polies.size(); i++)
    {
        for (const poly_vert& v : polies[i]->vertices)
        {
            res.vertices.push_back(v);
        }

        for (const poly_edge& e : polies[i]->edges)
        {
            res.edges.push_back(poly_edge(e.v0 + nVertices, e.v1 + nVertices));
        }

        for (const poly_face& f : polies[i]->faces)
        {
            res.faces.push_back(poly_face());
            poly_face& new_face = res.faces[res.faces.size() - 1];
            new_face.copy_properties(f);
            new_face.surface_group = f.surface_group + n_surface_groups;

            for (const poly_loop& loop : f.loops)
            {
                new_face.loops.push_back(poly_loop());
                poly_loop& new_loop = new_face.loops[new_face.loops.size() - 1];

                for (int v_i : loop.vertices)
                {
                    new_loop.vertices.push_back(v_i + nVertices);
                }
                new_loop.copy_properties(loop);
            }
        }

        nVertices += polies[i]->vertices.size();

        for (int s_i = 0; s_i < polies[i]->surface_groups.size(); s_i++)
            res.surface_groups.push_back(polies[i]->surface_groups[s_i]);

        n_surface_groups += polies[i]->surface_groups.size();

        res.bbox.addInternalBox(polies[i]->bbox);
    }

    res.deduplicate_edges_vertices_accelerated();

    //PRINT_TIMER(combine_poly_accelerated)
}

void combine_polyfolds_linear(const std::vector<polyfold*>& polies, polyfold& res)
{
    int n_surface_groups = 0;

    for (int j = 0; j < polies.size(); j++)
    {
        const polyfold& pf = *polies[j];

        for (int s_i = 0; s_i < pf.surface_groups.size(); s_i++)
            res.surface_groups.push_back(pf.surface_groups[s_i]);

        for (int f_i = 0; f_i < pf.faces.size(); f_i++)
        {
            res.faces.push_back(poly_face());
            poly_face& f = res.faces[res.faces.size() - 1];
            f.copy_properties(pf.faces[f_i]);
            f.surface_group = pf.faces[f_i].surface_group + n_surface_groups;

            for (const poly_loop& loop : pf.faces[f_i].loops)
            {
                f.loops.push_back(poly_loop());
                poly_loop& new_loop = f.loops[f.loops.size() - 1];

                for (int v_i : loop.vertices)
                {
                    int v = res.get_point_or_add(pf.vertices[v_i].V);
                    new_loop.vertices.push_back(v);
                    f.addVertex(v);
                }

                std::vector<int> tempv = loop.vertices;
                tempv.push_back(tempv[0]);

                for (int i = 0; i < tempv.size() - 1; i++)
                {
                    core::vector3df v0 = pf.vertices[tempv[i]].V;
                    core::vector3df v1 = pf.vertices[tempv[i + 1]].V;
                    int new_e = res.get_edge_or_add(res.get_point_or_add(v0), res.get_point_or_add(v1), 0);
                    f.addEdge(new_e);
                }

                new_loop.type = loop.type;
                new_loop.topo_group = loop.topo_group;
                
                new_loop.depth = loop.depth;
                new_loop.direction = loop.direction;
                res.calc_loop_bbox(f, new_loop);
            }
        }

        n_surface_groups += pf.surface_groups.size();

        res.bbox.addInternalBox(pf.bbox);
    }

    res.build_edges_BVH();

}

//currently unused
/*
void polyfold::merge_faces()
{
    int n_faces = this->faces.size();

    if(n_faces == 0)
        return;

    for(int f_i = 0; f_i <this->faces.size()-1; f_i++)
        for(int f_j = f_i+1; f_j <this->faces.size(); f_j++)
    {

        core::plane3df f_plane = core::plane3df(this->faces[f_i].m_center,this->faces[f_i].m_normal);

        if(is_parallel_normal(this->faces[f_i].m_normal,this->faces[f_j].m_normal)
           && this->faces[f_i].m_normal.dotProduct(this->faces[f_j].m_normal) > 0
           && is_coplanar_point(f_plane,this->faces[f_j].m_center))
        {
            for(int p_j=0; p_j< this->faces[f_j].loops.size(); p_j++ )
                this->faces[f_i].loops.push_back(this->faces[f_j].loops[p_j]);

            this->faces[f_j].loops.clear();
        }
    }

    std::vector<poly_face> new_faces;
    for(int f_i = 0; f_i <this->faces.size(); f_i++)
    {
        if(this->faces[f_i].loops.size()>0)
            new_faces.push_back(this->faces[f_i]);
    }
    this->faces = new_faces;

    int n_faces_merged = n_faces - this->faces.size();

    if(n_faces_merged>0) std::cout<<" merged "<<n_faces_merged<<" faces\n";

    polyfold positive;
    polyfold negative;

    negative.vertices = this->vertices;
    positive.vertices = this->vertices;

    for(int f_i=0;f_i<faces.size();f_i++)
    {
        poly_face f;
        poly_face f_p;

        f.m_center          = faces[f_i].m_center;
        f.m_normal          = faces[f_i].m_normal;
        f.bFlippedNormal    = faces[f_i].bFlippedNormal;

        f_p.m_center        = faces[f_i].m_center;
        f_p.m_normal        = faces[f_i].m_normal;
        f_p.bFlippedNormal  = faces[f_i].bFlippedNormal;

        for(int p=0;p<faces[f_i].loops.size();p++)
        {
            if(faces[f_i].loops[p].topo_group == LOOP_HOLLOW)
                {
                    f.loops.push_back(faces[f_i].loops[p]);
                    faces[f_i].loops[p].vertices.clear();
                }
        }
        negative.faces.push_back(f);

        for(int p=0;p<faces[f_i].loops.size();p++)
        {
            if(faces[f_i].loops[p].topo_group == LOOP_SOLID)
                {
                    f_p.loops.push_back(faces[f_i].loops[p]);
                    faces[f_i].loops[p].vertices.clear();
                }
        }
        positive.faces.push_back(f_p);
    }

    negative.reduce_edges_vertices();
    positive.reduce_edges_vertices();

    for(int f_i=0;f_i<faces.size();f_i++)
    {
        positive.meld_loops(f_i,false);
    }

    for(int f_i=0;f_i<faces.size();f_i++)
    {
       // negative.meld_loops(f_i);
    }

    for(int f_i=0;f_i<faces.size();f_i++)
    {
        for(int p=0;p<negative.faces[f_i].loops.size();p++)
        {
            poly_loop loop;
            for(int v_i=0;v_i<negative.faces[f_i].loops[p].vertices.size();v_i++)
            {
                core::vector3df V = negative.vertices[negative.faces[f_i].loops[p].vertices[v_i]].V;
                int v = this->get_point_or_add(V);
                loop.vertices.push_back(v);
            }
            loop.topo_group=LOOP_HOLLOW;

            //this->calc_loop_bbox(positive.faces[f_i],loop);
            this->faces[f_i].loops.push_back(loop);
        }

        for(int p=0;p<positive.faces[f_i].loops.size();p++)
        {
            poly_loop loop;
            for(int v_i=0;v_i<positive.faces[f_i].loops[p].vertices.size();v_i++)
            {
                core::vector3df V = positive.vertices[positive.faces[f_i].loops[p].vertices[v_i]].V;
                int v = this->get_point_or_add(V);
                loop.vertices.push_back(v);
            }
            if(positive.faces[f_i].loops[p].type == LOOP_INNER)
                loop.topo_group=LOOP_SOLID;
            else
                loop.topo_group=LOOP_HOLLOW;

            //this->calc_loop_bbox(positive.faces[f_i],loop);
            this->faces[f_i].loops.push_back(loop);

        }
    }
    for(int f_i=0;f_i<faces.size();f_i++)
    {
        for(int p=0;p<faces[f_i].loops.size();p++)
        {
           // if(faces[f_i].loops[p].topo_group == LOOP_SOLID)
                this->set_loop_solid(f_i,p);
           // else
           //     this->set_loop_open(f_i,p);
        }
    }

    reduce_edges_vertices();

    this->recalc_bbox();
}*/

//=======================================================================================================



int polyfold::get_edge_loop_no(int f_i, int e_i) const
{
    int v_0 = edges[e_i].v0;
    int v_1 = edges[e_i].v1;

    for(int p_i = 0 ; p_i < faces[f_i].loops.size(); p_i ++)
        for(int v_i : faces[f_i].loops[p_i].vertices)
            if(v_i == v_0 || v_i == v_1)
                return p_i;
    return -1;
}

std::vector<int> polyfold::get_vert_loop_no(int f_i, int my_v) const
{
    std::vector<int> ret;
    for(int p_i = 0 ; p_i < faces[f_i].loops.size(); p_i ++)
        for(int v_i : faces[f_i].loops[p_i].vertices)
            if(v_i == my_v)
                ret.push_back(p_i);
    return ret;
}


bool polyfold::get_point_in_loop(int face_i, int loop_i, core::vector3df& out, LineHolder& graph) const
{
    for (int i = 0; i < faces[face_i].loops[loop_i].vertices.size(); i++)
    {
        int v_0 = faces[face_i].loops[loop_i].vertices[i];
        int v_1 = i + 1 < faces[face_i].loops[loop_i].vertices.size() ?
            faces[face_i].loops[loop_i].vertices[i + 1] : faces[face_i].loops[loop_i].vertices[0];

        core::vector3df v0 = vertices[v_0].V;
        core::vector3df v1 = vertices[v_1].V;

        core::vector3df r = v1 - v0;

        core::vector3df v3 = r.crossProduct(faces[face_i].getOrientingNormal());
        v3.normalize();
        v3 *= 0.25;

        core::vector3df ret;

        ret = v0 + (v1 - v0) * 0.5 - v3;

        if (is_point_in_loop(face_i, loop_i,ret))
        {
            out = ret;
            return true;
        }
        //graph.points.push_back(ret);
        //graph.lines.push_back(core::line3df(vertices[v_0].V, vertices[v_1].V));
    }

    return false;
}

bool polyfold::get_facial_point(int face_i, int loop_i, core::vector3df& out, LineHolder& graph) const
{
    for (int i = 0; i < faces[face_i].loops[loop_i].vertices.size(); i++)
    {
        int v_0 = faces[face_i].loops[loop_i].vertices[i];
        int v_1 = i + 1 < faces[face_i].loops[loop_i].vertices.size() ?
            faces[face_i].loops[loop_i].vertices[i + 1] : faces[face_i].loops[loop_i].vertices[0];

        core::vector3df v0 = vertices[v_0].V;
        core::vector3df v1 = vertices[v_1].V;

        core::vector3df r = v1 - v0;

        core::vector3df v3 = r.crossProduct(faces[face_i].getOrientingNormal());
        v3.normalize();
        v3 *= 0.25;

        core::vector3df ret;

        if(is_closed_loop(face_i,loop_i))
            ret = v0 + (v1 - v0) * 0.5 - v3;
        else
            ret = v0 + (v1 - v0) * 0.5 + v3;

        if (is_point_on_face(face_i, ret))
        {
            out = ret;
            return true;
        }
        //graph.points.push_back(ret);
        //graph.lines.push_back(core::line3df(vertices[v_0].V, vertices[v_1].V));
    }

    return false;
}

//This function is known to fail in some cases (tiny slivers)
bool polyfold::get_facial_point(int face_i, int edge_i, int loop_i, core::vector3df & out, LineHolder &graph) const
{
    int ev_0 = this->edges[edge_i].v0;
    int ev_1 = this->edges[edge_i].v1;

    int v_0=-1;
    int v_1=-1;
    int n = this->faces[face_i].loops[loop_i].vertices.size()-1;

    if(this->faces[face_i].loops[loop_i].vertices[0] == ev_0 && this->faces[face_i].loops[loop_i].vertices[n] == ev_1)
    {
     v_0 = ev_1;
     v_1 = ev_0;
    }
    else if(this->faces[face_i].loops[loop_i].vertices[0] == ev_1 && this->faces[face_i].loops[loop_i].vertices[n] == ev_0)
    {
     v_0 = ev_0;
     v_1 = ev_1;
    }
    else
    {
        for (int i = 0; i < n; i++)
        {
            if (this->faces[face_i].loops[loop_i].vertices[i] == ev_0 && this->faces[face_i].loops[loop_i].vertices[i + 1] == ev_1)
            {
                v_0 = ev_0;
                v_1 = ev_1;
            }
            else if (this->faces[face_i].loops[loop_i].vertices[i] == ev_1 && this->faces[face_i].loops[loop_i].vertices[i + 1] == ev_0)
            {
                v_0 = ev_1;
                v_1 = ev_0;
            }
        }
    }

    if(v_0==-1)
    {
        std::cout<<"*error* get_facial_point, could not find edge on loop\n";
        return false;
    }

    core::vector3df v0 = this->vertices[v_0].V;
    core::vector3df v1 = this->vertices[v_1].V;

    core::vector3df r = v1-v0;

    core::vector3df v3 = r.crossProduct(this->faces[face_i].getOrientingNormal());
    v3.normalize();
    v3*=0.25;

    core::vector3df ret = v0+ (v1-v0)*0.5 - v3;

    graph.points.push_back(ret);
    graph.lines.push_back(core::line3df(this->vertices[v_0].V,this->vertices[v_1].V));

    out = ret;
    return true;
}

/*
std::vector<int> polyfold::get_all_loop_edges(int f_i, int p_i)
{
    std::vector<int> v_list;
    v_list.push_back(this->faces[f_i].loops[p_i].vertices[this->faces[f_i].loops[p_i].vertices.size()-1]);
    for(int v_i : this->faces[f_i].loops[p_i].vertices)
    {
        v_list.push_back(v_i);
    }
    std::vector<int> ret;
    for(int i=0;i<v_list.size()-1;i++)
    {
        int e = this->find_edge(v_list[i],v_list[i+1]);
        if(e != -1)
            ret.push_back(e);
    }
    return ret;
}*/

int polyfold::edge_classification(int edge)
{
    if (edges[edge].conv == EDGE_UNDEF)
        classify_edge<false>(edge);

    return edges[edge].conv;
}
