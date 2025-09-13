
#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"
#include "tolerances.h"


//#define ACCELERATED_PROPAGATION

using namespace irr;

/*
void polyfold::propagate_topo_group(int e_i, int v0)
{
    int v1 = this->get_opposite_end(e_i,v0);
    bool safe=true;

    std::vector<int> c_edges;

#ifdef ACCELERATED_PROPAGATION
    this->get_all_edges_from_point_accelerated(e_i, v1, c_edges);
#else
    this->get_all_edges_from_point(e_i, v1, c_edges);
#endif

    for(int e_j : c_edges)
        {
            if(this->edges[e_j].topo_group == 3)
                safe=false;
        }

    if(safe)
    {
        for(int e_j : c_edges)
            if(this->edges[e_j].topo_group == 2)
            {
                this->edges[e_j].topo_group = this->edges[e_i].topo_group;
                this->propagate_topo_group(e_j,v1);
            }
    }
}*/
/*
bool polyfold::apply_topology_groups(polyfold& pf2, int default_group, LineHolder& graph)
{
    for(int e_i = 0; e_i<this->edges.size(); e_i++)
        {
            if(this->edges[e_i].topo_group==0 || this->edges[e_i].topo_group==1)
            {
                this->propagate_topo_group(e_i,this->edges[e_i].v0);
                this->propagate_topo_group(e_i,this->edges[e_i].v1);
            }
        }

    bool found_one;
    do
    {
        found_one=false;
        for(int e_i = 0; e_i<this->edges.size(); e_i++)
            {
                if(this->edges[e_i].topo_group==2)
                {
                    found_one=true;

                    core::vector3df v0 = this->getVertex(e_i,0).V;
                    core::vector3df v1 = this->getVertex(e_i,1).V;

                    int RES = pf2.classify_point(v0+(v1-v0)*0.5,graph);

                    //graph.points.push_back(v0+(v1-v0)*0.5);

                    if(RES == TOP_FRONT)
                    {
                        this->edges[e_i].topo_group=0;
                    }
                    else if(RES == TOP_BEHIND)
                    {
                        this->edges[e_i].topo_group=1;
                    }
                    else if(RES == TOP_UNDEF)
                    {
                        if(pf2.topology == TOP_CONCAVE)
                        {
                            this->edges[e_i].topo_group = TOP_BEHIND;
                        }
                        else if(pf2.topology == TOP_CONVEX)
                        {
                            this->edges[e_i].topo_group = TOP_FRONT;
                        }
                    }

                    this->propagate_topo_group(e_i,this->edges[e_i].v0);
                    this->propagate_topo_group(e_i,this->edges[e_i].v1);
                }
            }
    } while(found_one==true);

    return true;
}
*/
//currently unused
void polyfold::simplify_edges()
{
   // std::cout<<"simplifying edges...\n";
    simp_try_again:

    for(int f_i=0;f_i<this->faces.size();f_i++)
    {
        for(int p_i = 0; p_i< this->faces[f_i].loops.size(); p_i++)
        {
            std::vector<int> tempv = this->faces[f_i].loops[p_i].vertices;
            tempv.push_back(tempv[0]);
            tempv.push_back(tempv[1]);

            for(int i=0; i < tempv.size()-2; i++)
            {
                core::line3df line(this->vertices[tempv[i]].V,this->vertices[tempv[i+2]].V);
                core::vector3df r = line.getClosestPoint(this->vertices[tempv[i+1]].V);
                f32 d = r.getDistanceFrom(this->vertices[tempv[i+1]].V);
                if(d<0.01)
                {
                    int mid = (i+1)%this->faces[f_i].loops[p_i].vertices.size();
                   // std::cout<<"remove "<<mid<<" from loop "<<p_i<<"\n";

                    std::vector<int> new_loop;
                    for(int j=0; j<this->faces[f_i].loops[p_i].vertices.size(); j++)
                    {
                        if(j != mid)
                            new_loop.push_back(this->faces[f_i].loops[p_i].vertices[j]);
                    }
                    this->faces[f_i].loops[p_i].vertices = new_loop;

                    goto simp_try_again;
                }
            }
        }
    }
}

void polyfold::reduce_edges_vertices_accelerated(const polyfold& pf, LineHolder& graph)
{
    std::vector<poly_vert> new_verts;
    std::vector<poly_edge> new_edges;

    const BVH_structure_simple<poly_vert>& bvh_vert_other = pf.vertices_BVH;
    const BVH_structure_pf<poly_edge>& bvh_edge_other = pf.edges_BVH;

    BVH_structure_simple<poly_vert> bvh_vert_self = pf.vertices_BVH;
    BVH_structure_pf<poly_edge> bvh_edge_self = pf.edges_BVH;

    for (int i = 0; i < bvh_vert_self.node_count; i++)
    {
        BVH_node& node = bvh_vert_self.nodes[i];
        if (node.isLeafNode())
            node.n_prims = 0;
    }

    for (int i = 0; i < bvh_edge_self.node_count; i++)
    {
        BVH_node& node = bvh_edge_self.nodes[i];
        if (node.isLeafNode())
            node.n_prims = 0;

    }

    for (int f_i = 0; f_i < this->faces.size(); f_i++)
    {
        this->faces[f_i].edges.clear();
        this->faces[f_i].vertices.clear();

        for (int p_i = 0; p_i < this->faces[f_i].loops.size(); p_i++)
        {
            for (int& v_i : this->faces[f_i].loops[p_i].vertices)
            {
                u16 node_id = bvh_vert_other.find_vertex_node_nocheck(vertices[v_i].V);
                if (bvh_vert_self.nodes[node_id].n_prims == 0)
                {
                    bvh_vert_self.nodes[node_id].n_prims = 1;

                    u16 prim_id = bvh_vert_other.indices[
                        bvh_vert_other.nodes[node_id].first_prim];

                    new_verts.push_back(poly_vert(vertices[prim_id].V));

                    v_i = new_verts.size() - 1;

                    bvh_vert_self.indices[
                        bvh_vert_other.nodes[node_id].first_prim] = v_i;

                    faces[f_i].addVertex(v_i);     
                }
                else
                {
                    v_i = bvh_vert_self.indices[
                        bvh_vert_other.nodes[node_id].first_prim];
                    faces[f_i].addVertex(v_i);
                }
            }
            //std::cout << f_i << " " << p_i << ": ";
            for (int i = 0; i < faces[f_i].loops[p_i].vertices.size(); i++)
            {
                
                int v_i0 = faces[f_i].loops[p_i].vertices[i];
                int v_i1;

                if (i + 1 < faces[f_i].loops[p_i].vertices.size())
                    v_i1 = faces[f_i].loops[p_i].vertices[i + 1];
                else
                    v_i1 = faces[f_i].loops[p_i].vertices[0];

                const poly_vert& v0 = new_verts[v_i0];
                const poly_vert& v1 = new_verts[v_i1];

                aabb_struct aabb;

                v0.grow(&aabb);
                v1.grow(&aabb);
                /*
                if (f_i == 2 && i == 2)
                {
                   // aabb.textdump();
                   // aabb.addDrawLines(graph);
                   // graph.points.push_back(v0.V);
                    //std::cout << v0.V.X << "," << v0.V.Y << "," << v0.V.Z << "," << "\n";
                   // std::cout << v1.V.X << "," << v1.V.Y << "," << v1.V.Z << "," << "\n";
                   // graph.points.push_back(v1.V);
                }*/
                //std::cout << "---\n";
                std::vector<u16> hits;
                bvh_edge_other.intersect_return_nodes(aabb, hits);
                for (int j=0;j<hits.size();j++)
                {
                    u16 n_i = hits[j];

                    u16 e_i = bvh_edge_other.indices[
                        bvh_edge_other.nodes[n_i].first_prim];
                    /*
                    if (f_i == 2 && i == 2)
                    {
                        //std::cout << getVertex(e_i, 0).V.X << "," << getVertex(e_i, 0).V.Y << "," << getVertex(e_i, 0).V.Z << "," << "   ";
                        //std::cout << getVertex(e_i, 1).V.X << "," << getVertex(e_i, 1).V.Y << "," << getVertex(e_i, 1).V.Z << "," << "\n";
                        //std::cout << e_i << ",";
                       // addDrawLinesEdgesByIndex(std::vector<u16>{e_i},graph);
                        //bvh_edge_other.addDrawLinesByIndex(std::vector<u16>{n_i},graph);
                       
                    }
                    //addDrawLinesEdgesByIndex(std::vector<u16>{79}, graph);
                    */
                    if (
                        ((is_same_point(getVertex(e_i, 0).V, v0.V) && is_same_point(getVertex(e_i, 1).V, v1.V))
                            || (is_same_point(getVertex(e_i, 1).V, v0.V) && is_same_point(getVertex(e_i, 0).V, v1.V))
                            )
                        && edges[e_i].topo_group != -1)
                    {
                        if (bvh_edge_self.nodes[n_i].n_prims == 0)
                        {
                            new_edges.push_back(poly_edge(v_i0, v_i1));

                            bvh_edge_self.indices[
                                bvh_edge_other.nodes[n_i].first_prim] = new_edges.size() - 1;

                            faces[f_i].edges.push_back(new_edges.size() - 1);

                            bvh_edge_self.nodes[n_i].n_prims = 1;
                        }
                        else
                        {
                            int e_i2 = bvh_edge_self.indices[
                                bvh_edge_other.nodes[n_i].first_prim];

                            faces[f_i].edges.push_back(e_i2);
                        }

                    }
                }
            }
        }
    }

    vertices = new_verts;
    edges = new_edges;
}

void polyfold::reduce_edges_vertices()
{
    int v_size = this->vertices.size();
    int e_size = this->edges.size();

    std::vector<poly_vert> old_verts = this->vertices;
    std::vector<int> new_verts;
    new_verts.assign(old_verts.size(),-1);

    this->edges.clear();
    this->vertices.clear();

    for(int f_i=0;f_i<this->faces.size();f_i++)
    {
        this->faces[f_i].edges.clear();
        this->faces[f_i].vertices.clear();

        std::vector<poly_loop> new_loops;
        for(int p_i = 0; p_i< this->faces[f_i].loops.size(); p_i++)
        {
            if(this->faces[f_i].loops[p_i].vertices.size()>2)
                new_loops.push_back(this->faces[f_i].loops[p_i]);
        }

        this->faces[f_i].loops = new_loops;

        for(int p_i = 0; p_i< this->faces[f_i].loops.size(); p_i++)
        {
            std::vector<int> tempv = this->faces[f_i].loops[p_i].vertices;
            tempv.push_back(tempv[0]);
            for (int i = 0; i < tempv.size() - 1; i++)
            {
                int v0 = this->get_point_or_add(old_verts[tempv[i]].V);
                new_verts[tempv[i]] = v0;

                int v1 = this->get_point_or_add(old_verts[tempv[i + 1]].V);
                new_verts[tempv[i+1]] = v1;

                this->faces[f_i].addVertex(v0);
                this->faces[f_i].addVertex(v1);

                int e_i = this->get_edge_or_add(v0, v1, 0);
                this->faces[f_i].addEdge(e_i);
            }

            std::vector<int> new_loop;

            for(int i=0; i < this->faces[f_i].loops[p_i].vertices.size(); i++)
            {
                new_loop.push_back(this->find_point(old_verts[this->faces[f_i].loops[p_i].vertices[i]].V));
            }

            this->faces[f_i].loops[p_i].vertices = new_loop;
        }
    }

    for (surface_group& sfg : this->surface_groups)
    {
        for (u16& v : sfg.c_brush.vertices)
        {
            if (new_verts[v] == -1)
                v = get_point_or_add(old_verts[v].V);
            else
                v = new_verts[v];
        }
    }
   // std::<<"edges reduced from "<<e_size<<" to "<<this->edges.size()<<", vertices reduced from "<<v_size<<" to "<<this->vertices.size()<<"\n";
}


bool polyfold::is_inner_loop(int f_i, int p_i, int p_j)
{
    if(this->faces[f_i].loops[p_i].vertices.size() == 0 ||
       this->faces[f_i].loops[p_j].vertices.size() == 0)
        return false;

    std::vector<int> tempv = this->faces[f_i].loops[p_i].vertices;
    tempv.push_back(tempv[0]);

    for(int i=0; i < tempv.size()-1; i++)
        {
            bool b = false;
            core::vector3df v0 = this->vertices[tempv[i]].V;
            core::vector3df v1 = this->vertices[tempv[i+1]].V;
            core::vector3df V = v0+(v1-v0)*0.5;

            if(this->is_point_in_loop(f_i,p_j,V))
                b= true;

            if(!b)
            {
                return false;
            }
        }

    for(int  v_i : this->faces[f_i].loops[p_i].vertices)
        for(int v_j : this->faces[f_i].loops[p_j].vertices)
            if(is_same_point(this->vertices[v_i].V,this->vertices[v_j].V))
               return false;

    return true;
}
