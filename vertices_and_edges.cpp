#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"
#include "tolerances.h"
#include "BVH.h"

using namespace irr;


void sort_inline_vertices(polyfold& pf)
{
    if (pf.vertices.size() < 3) return;
    std::vector<poly_vert> rvec;

    f32 d = 0;
    int res = -1;
    for (int i = 0; i < pf.vertices.size(); i++)
        for (int j = i + 1; j < pf.vertices.size(); j++)
        {
            core::vector3df r = pf.vertices[i].V - pf.vertices[j].V;
            if (r.getLength() > d)
            {
                res = i;
                d = r.getLength();
            }
        }
    core::vector3df origin = pf.vertices[res].V;
    d = 0;
    f32 sofar = -10;
    res = -1;
    for (int i = 0; i < pf.vertices.size(); i++)
    {
        d = 99999;
        for (int j = 0; j < pf.vertices.size(); j++)
        {
            core::vector3df r = pf.vertices[j].V - origin;
            if (r.getLength() > sofar && r.getLength() < d)
            {
                res = j;
                d = r.getLength();
            }
        }
        rvec.push_back(pf.vertices[res]);
        sofar = d;
    }
    pf.vertices = rvec;
}

int polyfold::classify_point_edge(int edge, core::vector3df v0, core::vector3df v1)
{
    bool bIsBehindConvex = true;
    bool bIsFrontConvex = false;
    bool bIsFrontConcave = true;
    bool bIsBehindConcave = false;

    if (this->edge_classification(edge) == TOP_CONVEX)
    {
        for (int f_i = 0; f_i < this->faces.size(); f_i++)
        {
            for (int e_i : this->faces[f_i].edges)
            {
                if (e_i == edge && this->classify_point_face(f_i, v1) != TOP_BEHIND)
                    bIsBehindConvex = false;
                if (e_i == edge && this->classify_point_face(f_i, v1) == TOP_FRONT)
                {
                    bIsFrontConvex = true;
                }
            }
        }
        if (bIsBehindConvex)
            return TOP_BEHIND;
        else if (bIsFrontConvex)
            return TOP_FRONT;
        else return TOP_UNDEF;
    }
    else if (this->edge_classification(edge) == TOP_CONCAVE)
    {
        for (int f_i = 0; f_i < this->faces.size(); f_i++)
        {
            for (int e_i : this->faces[f_i].edges)
            {
                if (e_i == edge && this->classify_point_face(f_i, v1) != TOP_FRONT)
                    bIsFrontConcave = false;
                if (e_i == edge && this->classify_point_face(f_i, v1) == TOP_BEHIND)
                    bIsBehindConcave = true;
            }
        }
        if (bIsBehindConcave)
            return TOP_BEHIND;
        else if (bIsFrontConcave)
            return TOP_FRONT;
        else return TOP_UNDEF;
    }

    return TOP_UNDEF;
}


int polyfold::classify_point_face(int f_i, core::vector3df v)
{
    core::plane3df plane = core::plane3df(this->faces[f_i].m_center, this->faces[f_i].m_normal);

    if (plane.getDistanceTo(v) > POINT_IS_COPLANAR_DIST)
        return TOP_FRONT;
    else if (plane.getDistanceTo(v) < -POINT_IS_COPLANAR_DIST)
        return TOP_BEHIND;
    else return TOP_UNDEF;
}

int polyfold::classify_point(int face_i, core::vector3df v0, core::vector3df v1, LineHolder& graph)
{
    int RES = TOP_UNDEF;

    std::vector<int> convex_edges;
    std::vector<int> concave_edges;
    std::vector<int> undef_edges;

    int v_i = this->find_point(v0);

    if (v_i != -1)
    {
        for (int e_i = 0; e_i < this->edges.size(); e_i++)
        {
            if (this->edges[e_i].topo_group != -1 && (this->edges[e_i].v0 == v_i || this->edges[e_i].v1 == v_i))
            {
                if (this->edge_classification(e_i) == EDGE_CONVEX)
                    convex_edges.push_back(e_i);
                else if (this->edge_classification(e_i) == EDGE_CONCAVE)
                    concave_edges.push_back(e_i);
                else if (this->edge_classification(e_i) == EDGE_COMPLEX)
                {
                    undef_edges.push_back(e_i);
                }
            }
        }
    }
    else
    {
        for (int e_i = 0; e_i < this->edges.size(); e_i++)
        {
            if (this->edges[e_i].topo_group != -1 && this->point_is_on_edge(e_i, v0))
            {
                if (this->edge_classification(e_i) == EDGE_CONVEX)
                    convex_edges.push_back(e_i);
                else if (this->edge_classification(e_i) == EDGE_CONCAVE)
                    concave_edges.push_back(e_i);
                else if (this->edge_classification(e_i) == EDGE_COMPLEX)
                {
                    undef_edges.push_back(e_i);
                }
            }
        }
    }

    if (convex_edges.size() == 0 && concave_edges.size() == 0 && undef_edges.size() == 0)
    {
        return this->classify_point_face(face_i, v1);
    }
    else if (undef_edges.size() > 0 || (convex_edges.size() > 0 && concave_edges.size() > 0))
    {
        //COMPLEX POINT / EDGE
        graph.points.push_back(v0);
        //std::cout<<"complex\n";
        //std::vector<core::vector3df> slip_vecs = this->get_edge_slip_vectors(undef_edges[0]);
        //std::cout<<v0.X<<" "<<v0.Y<<" "<<v0.Z<<" to ";
        //std::cout<<v1.X<<" "<<v1.Y<<" "<<v1.Z<<"\n";
        std::vector<int> nfaces;
        std::vector<int> mfaces;
        std::vector<int> medges;


        for (int e_i : undef_edges)
            medges.push_back(e_i);
        for (int e_i : concave_edges)
            medges.push_back(e_i);
        for (int e_i : convex_edges)
            medges.push_back(e_i);

        for (int f_i = 0; f_i < this->faces.size(); f_i++)
        {
            for (int e_i : this->faces[f_i].edges)
            {
                for (int e_j : undef_edges)
                    if (e_i == e_j)
                        nfaces.push_back(f_i);
                for (int e_j : concave_edges)
                    if (e_i == e_j)
                        nfaces.push_back(f_i);
                for (int e_j : convex_edges)
                    if (e_i == e_j)
                        nfaces.push_back(f_i);
            }
        }

        for (int f_i : nfaces)
        {
            bool b = false;
            for (int f_j : mfaces)
                if (f_i == f_j)
                    b = true;
            if (!b)
                mfaces.push_back(f_i);
        }

        int ii = 0;
        //std::cout << " complex edge: ";
        while (RES == TOP_UNDEF && ii < medges.size())
        {
            int e_0 = medges[ii];

            std::vector<core::vector3df> slip_vecs;
           // std::cout << e_0 << " ";

            core::vector3df v2 = this->getVertex(e_0, 0).V - this->getVertex(e_0, 1).V;

            std::vector<int> bfaces;
            for (int f_i : mfaces)
            {
                for (int e_i : this->faces[f_i].edges)
                {
                    if (e_i == e_0)
                        bfaces.push_back(f_i);
                }
            }

            //std::cout<<bfaces.size()<<" faces for edge... "<<e_0<<"\n";

            for (int f_i : bfaces)
            {
                core::vector3df r = v2.crossProduct(this->faces[f_i].m_normal);
                r.normalize();

                if (this->is_point_on_face(f_i, this->getVertex(e_0, 1).V + v2 * 0.5 + r))
                    slip_vecs.push_back(r);
                else
                    slip_vecs.push_back(-r);
            }
            v2.normalize();

            //std::cout<<slip_vecs.size()<<" slip vecs\n";

           // std::cout<<v2.X<<","<<v2.Y<<","<<v2.Z<<" <-- \n";
           // for(int i=0;i<slip_vecs.size();i++)
           //     std::cout<<slip_vecs[i].X<<","<<slip_vecs[i].Y<<","<<slip_vecs[i].Z<<"\n";


//            graph.points.push_back(this->getVertex(e_0,1).V+v2*0.5+r);

               // std::cout<<"aiy\n";

            for (int i = 0; i < slip_vecs.size(); i++)
            {
                if (RES != TOP_UNDEF)
                    continue;

                core::vector3df w0 = v0 + v2 * 3 + slip_vecs[i] * 3;
                core::vector3df w1 = v1 + v2 * 3 + slip_vecs[i] * 3;
                //graph.lines.push_back(core::line3df(w0,w1));
                core::vector3df ipoint;
                f32 d = 9999;
                f32 dd;
                std::vector<f32> dist;
                std::vector<int> res;
                // std::cout<<"---\n";

                for (int f_i : mfaces)
                {
                    core::plane3df f_plane = core::plane3df(this->faces[f_i].m_center, this->faces[f_i].m_normal);
                    if (f_plane.getIntersectionWithLine(w1, v0 - v1, ipoint) && this->is_point_on_face(f_i, ipoint))
                    {
                        w1 = v1 + v2 * 10 + slip_vecs[i] * 10;
                        f_plane.getIntersectionWithLine(w1, v0 - v1, ipoint);

                        //RES = this->classify_point_face(f_i,w1);
                        dd = ipoint.getDistanceFrom(w1);
                        dist.push_back(ipoint.getDistanceFrom(w1));
                        res.push_back(this->classify_point_face(f_i, w1));
                        //std::cout<<"got a point! ("<<f_i<<") "<<dd<<", RES = "<<this->classify_point_face(f_i,w1)<<"\n";

                    }
                }
                if (res.size() > 1)
                {
                    double smallest = 9999;
                    int smallest_i;
                    for (int i = 0; i < res.size(); i++)
                    {
                        if (dist[i] < smallest)
                        {
                            smallest_i = i;
                            smallest = dist[i];
                        }
                    }
                    double dif = 9999;
                    for (int i = 0; i < res.size(); i++)
                    {
                        if (i != smallest_i)
                            dif = std::min(fabs(smallest - dist[i]), dif);
                    }
                    //std::cout<<"dif="<<dif<<"\n";
                    if (dif > 0.01)
                    {
                        f32 d = 9999;
                        for (int i = 0; i < res.size(); i++)
                        {
                            if (dist[i] < d)
                            {
                                d = dist[i];
                                RES = res[i];
                            }
                        }
                    }
                }
                else if (res.size() == 1)
                {
                    //std::cout<<"* ";
                    RES = res[0];
                }
            }
            ii++;
        }

        //std::cout<<" RES = "<<RES<<"\n";
        return RES;
    }

    bool bIsBehindConvex = true;
    bool bIsFrontConvex = false;
    bool bIsFrontConcave = true;
    bool bIsBehindConcave = false;

    if (convex_edges.size() > 0)
    {
        for (int e_i : convex_edges)
        {
            if (this->classify_point_edge(e_i, v0, v1) != TOP_BEHIND)
                bIsBehindConvex = false;
        }

        for (int e_i : convex_edges)
        {
            if (this->classify_point_edge(e_i, v0, v1) == TOP_FRONT)
            {
                bIsFrontConvex = true;
            }
        }

        if (bIsBehindConvex)
            RES = TOP_BEHIND;
        else if (bIsFrontConvex)
            RES = TOP_FRONT;
        else RES = TOP_UNDEF;
    }
    else if (concave_edges.size() > 0)
    {
        for (int e_i : concave_edges)
        {
            if (this->classify_point_edge(e_i, v0, v1) != TOP_FRONT)
                bIsFrontConcave = false;
        }

        for (int e_i : concave_edges)
        {
            if (this->classify_point_edge(e_i, v0, v1) == TOP_BEHIND)
                bIsBehindConcave = true;
        }

        if (bIsBehindConcave)
            RES = TOP_BEHIND;
        else if (bIsFrontConcave)
            RES = TOP_FRONT;
        else RES = TOP_UNDEF;
    }

    return RES;
}



int polyfold::classify_point(core::vector3df v0, LineHolder& graph)
{
    if (this->vertices.size() == 0)
        return TOP_UNDEF;

    for (int w = 0; w < this->vertices.size(); w++)
    {
        std::vector<core::vector3df> ipoints;
        std::vector<int> face_n;
        core::vector3df axis = v0 - this->vertices[w].V;

        for (int f_i = 0; f_i < this->faces.size(); f_i++)
        {
            const poly_face& f = this->faces[f_i];
            core::plane3df f_plane = core::plane3df(f.m_center, f.m_normal);
            core::vector3df ipoint;

            if (f_plane.getIntersectionWithLine(v0, axis, ipoint) && this->is_point_on_face(f_i, ipoint))
            {
                ipoints.push_back(ipoint);
                face_n.push_back(f_i);
            }
        }

        if (ipoints.size() >= 1)
        {
            int status = -1;

            int res;
            f32 d = 99999;
            for (int i = 0; i < ipoints.size(); i++)
            {
                f32 r = v0.getDistanceFrom(ipoints[i]);
                if (r < d && r>0.01)
                {
                    res = i;
                    d = r;
                }
            }

            int RES = TOP_UNDEF;

            LineHolder nograph;

            RES = this->classify_point(face_n[res], ipoints[res], v0, nograph);

            if (RES != TOP_UNDEF)
            {
                return RES;
            }
        }
    }
    return TOP_UNDEF;
}
