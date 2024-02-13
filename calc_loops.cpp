#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"
#include "tolerances.h"


//#define ACCELERATED_PROPAGATION

using namespace irr;

bool is_left_from(core::vector3df v0, core::vector3df v1, core::vector3df straight, core::vector3df N)
{
    core::vector3df cp0 = v0.crossProduct(straight);
    core::vector3df cp1 = v1.crossProduct(straight);

    if (cp0.dotProduct(N) < -0.001 && cp1.dotProduct(N) > 0.001)
        return false;
    else if (cp0.dotProduct(N) > 0.001 && cp1.dotProduct(N) < -0.001)
        return true;

    core::vector3df cp = v0.crossProduct(v1);
    if (cp.dotProduct(N) > 0.001)
        return true;
    return false;
}


std::vector<int> polyfold::get_edges_from_point(int f_i, int e_0, int v) const
{
    std::vector<int> ret;

    for (int e_i : faces[f_i].edges)
    {
        if ((edges[e_i].v0 == v || edges[e_i].v1 == v) && (e_i != e_0) && edges[e_i].topo_group != -1)
            ret.push_back(e_i);
    }

    return ret;
}

int polyfold::get_next_loop_edge_going_left(int f_i, int e_i, int v_i)
{
    int ret;
    std::vector<int> links_ = get_edges_from_point(f_i, e_i, v_i);

    std::vector<int> links;
    for (int i : links_)
        if (this->edges[i].p2 != -1)
            links.push_back(i);

    if (links.size() == 1)
    {
        ret = links[0];
    }
    else if (links.size() > 1)
    {
        std::vector<core::vector3df> vecs;
        for (int e_j : links)
        {
            int v_j = this->get_opposite_end(e_j, v_i);
            vecs.push_back(this->vertices[v_j].V - this->vertices[v_i].V);
        }
        core::vector3df straight_ahead = this->vertices[v_i].V - this->vertices[this->get_opposite_end(e_i, v_i)].V;

        for (int i = 0; i < links.size() - 1; i++)
            for (int j = i + 1; j < links.size(); j++)
            {
                if (is_left_from(vecs[j], vecs[i], straight_ahead, this->faces[f_i].getOrientingNormal()))
                {
                    core::vector3df tv = vecs[i];
                    vecs[i] = vecs[j];
                    vecs[j] = tv;

                    int ti = links[i];
                    links[i] = links[j];
                    links[j] = ti;
                }
            }

        ret = links[0];
    }
    else
        ret = -1;

    return ret;
}

std::vector<int> polyfold::get_loop_going_left(int f_i, int v_0, int e_0, int v_1, LineHolder& graph)
{
    std::vector<int> e_loop;
    e_loop.push_back(e_0);
    //graph.lines.push_back(core::line3df(this->getVertex(e_0,0).V,this->getVertex(e_0,1).V));
    int v_i = v_1;
    int e_i = e_0;

    while (true)
    {
        e_i = get_next_loop_edge_going_left(f_i, e_i, v_i);

        if (e_i == -1)
        {
            std::cout << "*Error calc loops search space zero\n";
            return e_loop;
        }

        if (e_i == e_0)
            return e_loop;

        v_i = this->get_opposite_end(e_i, v_i);

        e_loop.push_back(e_i);
    };
}




/*
void polyfold::copy_loops(polyfold& pf, int f_i)
{
    poly_face& f = faces[f_i];

    f.loops.clear();

    for (poly_loop loop : pf.faces[f_i].loops)
    {
        poly_loop new_loop;
        for (int v_i : loop.vertices)
        {
            int v = get_point_or_add(pf.vertices[v_i].V);
            new_loop.vertices.push_back(v);
            f.addVertex(v);
        }

        std::vector<int> tempv = loop.vertices;
        tempv.push_back(tempv[0]);

        for (int i = 0; i < tempv.size() - 1; i++)
        {
            core::vector3df v0 = pf.vertices[tempv[i]].V;
            core::vector3df v1 = pf.vertices[tempv[i + 1]].V;
            int new_e = get_edge_or_add(get_point_or_add(v0), get_point_or_add(v1), 0);
            f.addEdge(new_e);
        }

        new_loop.type = loop.type;
        new_loop.topo_group = loop.topo_group;
        new_loop.direction = loop.direction;
        calc_loop_bbox(f, new_loop);
        f.loops.push_back(new_loop);
    }

}*/

bool polyfold::is_clockwise_loop(int f_i, int p_i) const
{
    f32 testz = (this->faces[f_i].loops[p_i].max_z - this->faces[f_i].loops[p_i].min_z) * 0.5 + this->faces[f_i].loops[p_i].min_z;
    f32 testx = this->faces[f_i].loops[p_i].min_x - 10;
    int res = this->left_right_test(f_i, p_i, testx, testz);
    if (res == 0)
    {
        return true;
    }
    return false;
}
int polyfold::get_next_loop_edge_going_right(int f_i, int e_i, int v_i)
{
    int ret;
    std::vector<int> links_ = get_edges_from_point(f_i, e_i, v_i);

    std::vector<int> links;
    for (int i : links_)
        if (this->edges[i].p2 != -1)
            links.push_back(i);

    if (links.size() == 1)
    {
        ret = links[0];
    }
    else if (links.size() > 1)
    {
        std::vector<core::vector3df> vecs;
        for (int e_j : links)
        {
            int v_j = this->get_opposite_end(e_j, v_i);
            vecs.push_back(this->vertices[v_j].V - this->vertices[v_i].V);
        }

        core::vector3df straight_ahead = this->vertices[v_i].V - this->vertices[this->get_opposite_end(e_i, v_i)].V;

        for (int i = 0; i < links.size() - 1; i++)
            for (int j = i + 1; j < links.size(); j++)
            {
                if (is_left_from(vecs[j], vecs[i], straight_ahead, this->faces[f_i].getOrientingNormal()))
                {
                    core::vector3df tv = vecs[i];
                    vecs[i] = vecs[j];
                    vecs[j] = tv;

                    int ti = links[i];
                    links[i] = links[j];
                    links[j] = ti;
                }
            }
        ret = links[links.size() - 1];
    }
    else
        ret = -1;

    return ret;
}


std::vector<int> polyfold::get_loop_going_right(int f_i, int v_0, int e_0, int v_1, LineHolder& graph)
{
    std::vector<int> e_loop;
    e_loop.push_back(e_0);

    int v_i = v_1;
    int e_i = e_0;

    while (true)
    {
        e_i = get_next_loop_edge_going_right(f_i, e_i, v_i);

        if (e_i == -1)
        {
            std::cout << "*Error calc loops search space zero\n";
            return e_loop;
        }

        if (e_i == e_0)
            return e_loop;

        v_i = get_opposite_end(e_i, v_i);

        e_loop.push_back(e_i);
    };
}

std::vector<int> polyfold::search_connected_edges(polyfold& pf, int f_i, int e_0) const
{
    std::vector<int> ret;
    ret.push_back(e_0);

    int v0 = edges[e_0].v0;
    int v1 = edges[e_0].v1;

    std::vector<int> links = get_edges_from_point(f_i, e_0, v0);
    for (int e_i : links)
    {
        int w0 = pf.get_point_or_add(getVertex(e_i, 0).V);
        int w1 = pf.get_point_or_add(getVertex(e_i, 1).V);
        if (pf.find_edge(w0, w1) == -1)
        {
            int new_e = pf.get_edge_or_add(w0, w1, 0);
            std::vector<int> res = search_connected_edges(pf, f_i, e_i);
            for (int i : res)
                ret.push_back(i);
        }
    }

    links = get_edges_from_point(f_i, e_0, v1);
    for (int e_i : links)
    {
        int w0 = pf.get_point_or_add(getVertex(e_i, 0).V);
        int w1 = pf.get_point_or_add(getVertex(e_i, 1).V);
        if (pf.find_edge(w0, w1) == -1)
        {
            int new_e = pf.get_edge_or_add(w0, w1, 0);
            std::vector<int> res = search_connected_edges(pf, f_i, e_i);
            for (int i : res)
                ret.push_back(i);
        }
    }
    return ret;
}

std::vector<int> polyfold::get_all_connected_edges(int f_i, int e_0) const
{
    polyfold pf;
    int v0 = pf.get_point_or_add(getVertex(e_0, 0).V);
    int v1 = pf.get_point_or_add(getVertex(e_0, 1).V);
    pf.get_edge_or_add(v0, v1, 0);

    return search_connected_edges(pf, f_i, e_0);
}

void polyfold::do_loops(int f_i, int e_0, LineHolder& graph)
{
    LineHolder nograph;
    int v0 = this->edges[e_0].v0;
    int v1 = this->edges[e_0].v1;

    //std::cout<<"f "<<f_i<<" e "<<e_0<<"\n";

    //std::cout<<"e_0="<<e_0<<" v0="<<v0<<" v1="<<v1<<" ";
    if (this->edges[e_0].p2 == 0 || this->edges[e_0].p2 == 2)
    {
        //std::cout<<"a\n";
        std::vector<int> res = get_loop_going_right(f_i, v0, e_0, v1, nograph);
        //std::cout<<"right res: ";
        //for(int i=0;i<res.size();i++)
        //    std::cout<<res[i]<<" ";
        //std::cout<<"\n";
        this->add_loop_from_edges(f_i, res, false);
    }
    if (this->edges[e_0].p2 == 1)
    {
        //std::cout<<"b\n";
        std::vector<int> res = get_loop_going_left(f_i, v0, e_0, v1, nograph);
        //std::cout<<"left res: ";
        //for(int i=0;i<res.size();i++)
        //    std::cout<<res[i]<<" ";
        //std::cout<<"\n";
        this->add_loop_from_edges(f_i, res, true);
    }

    //0- unexplored
    //1- v0 to v1 going right = v1 to v0 going left
    //2- v1 to v0 going right = v0 to v1 going left
    //3- both
}

void polyfold::calc_loops(int f_i, LineHolder& graph)
{
    for (poly_edge& edge : this->edges)
        edge.p2 = -1;

    LineHolder nograph;

    this->faces[f_i].loops.clear();

    for (int e_0 : this->faces[f_i].edges)
    {
        if (this->edges[e_0].p2 == -1 && this->edges[e_0].topo_group != -1)
        {
            std::vector<int> evec = this->get_all_connected_edges(f_i, e_0);
            for (int e_i : evec)
            {
                this->edges[e_i].p2 = 0;
            }
        }
    }

    bool found_loose_end;
    int n_loose_ends = 0;

    do
    {
        found_loose_end = false;
        for (int e_0 : this->faces[f_i].edges)
        {
            if (this->edges[e_0].p2 != -1)
            {
                bool ok0 = false;
                bool ok1 = false;

                std::vector<int> evec0;
                std::vector<int> evec1;

                this->get_all_edges_from_point(e_0, this->edges[e_0].v0, evec0);
                this->get_all_edges_from_point(e_0, this->edges[e_0].v1, evec1);

                for (int e_i : evec0)
                {
                    if (this->edges[e_i].p2 != -1)
                        ok0 = true;
                }

                for (int e_i : evec1)
                {
                    if (this->edges[e_i].p2 != -1)
                        ok1 = true;
                }

                if (ok0 == false || ok1 == false)
                {
                    this->edges[e_0].p2 = -1;
                    found_loose_end = true;
                    n_loose_ends++;
                }
            }
        }
    } while (found_loose_end == true);

    //if (n_loose_ends > 0) std::cout << " Calc Loops: cleaned up " << n_loose_ends << " loose ends\n";

    for (int e_0 : this->faces[f_i].edges)
    {
        if (this->edges[e_0].p2 < 3 && this->edges[e_0].p2 != -1 && this->edges[e_0].topo_group != -1)
        {
            this->do_loops(f_i, e_0, graph);
        }
    }

    this->sort_loops(f_i);
}

void polyfold::add_loop_from_edges(int f_i, std::vector<int> e_vec, bool is_left)
{
    if (e_vec.size() > 0)
    {
        poly_loop loop;
        int v0;
        if (this->edges[e_vec[0]].v0 == this->edges[e_vec[1]].v0 || this->edges[e_vec[0]].v0 == this->edges[e_vec[1]].v1)
            v0 = this->edges[e_vec[0]].v1;
        else
            v0 = this->edges[e_vec[0]].v0;

        //0- unexplored
        //1- v0 to v1 going right = v1 to v0 going left
        //2- v1 to v0 going right = v0 to v1 going left
        //3- both explored

        for (int e_i : e_vec)
        {
            loop.vertices.push_back(v0);
            if (is_left == false) //going right
            {
                if (v0 == this->edges[e_i].v0) //v0 to v1
                {
                    if (this->edges[e_i].p2 == 0)
                        this->edges[e_i].p2 = 1;
                    else if (this->edges[e_i].p2 == 2)
                        this->edges[e_i].p2 = 3;
                }
                else //v1 to v0
                {
                    if (this->edges[e_i].p2 == 0)
                        this->edges[e_i].p2 = 2;
                    else if (this->edges[e_i].p2 == 1)
                        this->edges[e_i].p2 = 3;
                }
            }
            else //going left
            {
                if (v0 == this->edges[e_i].v0) //v0 to v1
                {
                    if (this->edges[e_i].p2 == 0)
                        this->edges[e_i].p2 = 2;
                    else if (this->edges[e_i].p2 == 1)
                        this->edges[e_i].p2 = 3;
                }
                else //v1 to v0
                {
                    if (this->edges[e_i].p2 == 0)
                        this->edges[e_i].p2 = 1;
                    else if (this->edges[e_i].p2 == 2)
                        this->edges[e_i].p2 = 3;
                }
            }
            v0 = get_opposite_end(e_i, v0);
        }

        this->faces[f_i].loops.push_back(loop);

        int new_p = this->faces[f_i].loops.size() - 1;

        this->calc_loop_bbox(f_i, new_p);
        if (fabs(this->faces[f_i].loops[new_p].max_x - this->faces[f_i].loops[new_p].min_x < 0.5) ||
            fabs(this->faces[f_i].loops[new_p].max_z - this->faces[f_i].loops[new_p].min_z < 0.5))
            std::cout << "*error bad loop " << f_i << "," << new_p << "\n";

        bool bCW = this->is_clockwise_loop(f_i, new_p);

        if ((bCW && is_left == false) || (!bCW && is_left == true))
            this->faces[f_i].loops[new_p].type = LOOP_INNER;
        else
            this->faces[f_i].loops[new_p].type = LOOP_OUTER;

        /*
        if(f_i==0)
        //if(false)
        {
        std::cout<<"f "<<f_i<<" loop "<<this->faces[f_i].loops.size()<<" ";

        if(is_left) std::cout<<" is_left=true ";
        else std::cout<<" is_left=false ";

        if(bCW) std::cout<<"bCW = true\n";
        else std::cout<<"bCW = false\n";
        }
        */

    }
}


//called from calc_loops as the last step
void polyfold::sort_loops(int f_i)
{
    poly_face* face = &this->faces[f_i];

    for (int i = 0; i < face->loops.size(); i++)
    {
        if (face->loops[i].vertices.size() > 0)
            this->set_loop_solid(f_i, i);
    }

    for (int i = 0; i < face->loops.size(); i++)
    {
        if (face->loops[i].type == LOOP_OUTER)
            face->loops[i].depth = 0;
        else
            face->loops[i].depth = -1;
    }

    //Check for nested loops
    for (int i = 0; i < face->loops.size(); i++)
        for (int j = i + 1; j < face->loops.size(); j++)
        {
            if (face->loops[i].vertices.size() == 0 || face->loops[j].vertices.size() == 0)
                continue;

            bool b1 = true;
            bool b2 = true;

            for (int v_i : face->loops[i].vertices)
                if (!this->is_point_in_loop(f_i, j, this->vertices[v_i].V))
                    b1 = false;
            for (int v_j : face->loops[j].vertices)
                if (!this->is_point_in_loop(f_i, i, this->vertices[v_j].V))
                    b2 = false;

            if (b1 && face->loops[j].type == LOOP_OUTER)
                face->loops[i].depth++;
            else if (b2 && face->loops[i].type == LOOP_OUTER)
                face->loops[j].depth++;
        }
    /*
    for (int i = 0; i < face->loops.size(); i++)
    {
        if (face->loops[i].type == LOOP_OUTER && face->loops[i].depth % 2 == 1)
        {
            face->loops[i].reverse();
            face->loops[i].topo_group = LOOP_HOLLOW;
        }
        else
            face->loops[i].topo_group = LOOP_SOLID;
    }*/
}
