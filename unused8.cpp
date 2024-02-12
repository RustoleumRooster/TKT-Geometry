
/*
void do_topology_groups(polyfold& pf4, polyfold& pf2, int rule, int rule2, LineHolder& graph)
{
    pf4.apply_topology_groups(pf2, rule2, graph);

    if(rule == GEO_SUBTRACT)
        for(int e_i = 0; e_i<pf4.edges.size(); e_i++)
        {
            if(pf4.edges[e_i].topo_group == 1 || pf4.edges[e_i].topo_group == 0)
            {
                pf4.edges[e_i].topo_group = !pf4.edges[e_i].topo_group;
            }
        }
}
*/


/*
void do_self_intersections(polyfold& pf, const std::vector<BVH_intersection_struct>& faces_faces)
{
    if(pf.faces.size()==0)
        return;

#ifdef BVH_OPTIMIZE
    for(BVH_intersection_struct hit: faces_faces)
    {
        u16 face_i = hit.X;
        u16 face_j = hit.Y;
#else
    for(int face_i=0;face_i<pf.faces.size()-1;face_i++)
        for(int face_j=face_i+1;face_j<pf.faces.size();face_j++)
    {
#endif
            poly_face& f=pf.faces[face_i];
            poly_face& f2=pf.faces[face_j];

            core::vector3df i_point;
            core::vector3df i_vec;

            core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
            core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);

            if(f_plane.getIntersectionWithPlane(f2_plane,i_point,i_vec) && is_parallel_normal(f.m_normal,f2.m_normal) == false)
            {
                for(int e_i : pf.faces[face_i].edges)
                {
                    bool b1 = false;
                    bool b2 = false;

                    for(int v_j: pf.faces[face_j].vertices)
                        if(v_j==pf.edges[e_i].v0)
                            b1=true;

                    for(int v_j: pf.faces[face_j].vertices)
                        if(v_j==pf.edges[e_i].v1)
                            b2=true;

                    if(b1 && b2)
                    {
                        core::vector3df v0 = pf.getVertex(e_i,0).V + (pf.getVertex(e_i,1).V - pf.getVertex(e_i,0).V)*0.5;

                        if(pf.is_point_on_face(face_j,v0))
                        {
                            pf.faces[face_j].addEdge(e_i);
                        }
                    }
                }

                for(int e_j : pf.faces[face_j].edges)
                {
                    bool b1 = false;
                    bool b2 = false;

                    for(int v_i: pf.faces[face_i].vertices)
                        if(v_i==pf.edges[e_j].v0)
                            b1=true;

                    for(int v_i: pf.faces[face_i].vertices)
                        if(v_i==pf.edges[e_j].v1)
                            b2=true;

                    if(b1 && b2)
                    {
                        core::vector3df v0 = pf.getVertex(e_j,0).V + (pf.getVertex(e_j,1).V - pf.getVertex(e_j,0).V)*0.5;

                        if(pf.is_point_on_face(face_i,v0))
                        {
                            pf.faces[face_i].addEdge(e_j);
                        }
                    }
                }
            }
        }
}
*/


/*
void do_common_topology(polyfold& pf, polyfold& pf2, polyfold& pf4, polyfold& pf5, const std::vector<BVH_intersection_struct>& faces_faces,LineHolder& graph)
{

#ifdef BVH_OPTIMIZE
    for(const BVH_intersection_struct& hit: faces_faces)
        {
            u16 face_i = hit.X;
            u16 face_j = hit.Y;
#else
    for(int face_i=0;face_i<pf.faces.size();face_i++)
        for(int face_j=0;face_j<pf2.faces.size();face_j++)
        {
#endif
            poly_face& f=pf.faces[face_i];
            poly_face& f2=pf2.faces[face_j];

            core::vector3df i_point;
            core::vector3df i_vec;

            core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
            core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);

            if(f_plane.getIntersectionWithPlane(f2_plane,i_point,i_vec) && is_parallel_normal(f.m_normal,f2.m_normal) == false)
            {
                polyfold verts;
                for(int v_i : pf4.faces[face_i].vertices)
                {
                    core::vector3df v = pf4.vertices[v_i].V;
                    if(is_coplanar_point(f2_plane,v) && pf5.is_point_on_face(face_j,v))
                    {
                        verts.get_point_or_add(v);
                        //graph.points.push_back(v);
                    }
                }

                for(int v_j : pf5.faces[face_j].vertices)
                {
                    core::vector3df v = pf5.vertices[v_j].V;
                    if(is_coplanar_point(f_plane,v) && pf.is_point_on_face(face_i,v))
                    {
                        verts.get_point_or_add(v);
                        //graph.points.push_back(v);
                    }
                }

                if(verts.vertices.size()>1)
                {
                    sort_inline_vertices(verts);
                    for(int i=0;i<verts.vertices.size()-1;i++)
                    {
                        //graph.points.push_back(verts.vertices[i+1].V);
                        graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));

                        core::vector3df r = verts.vertices[i].V + (verts.vertices[i+1].V - verts.vertices[i].V)*0.47;

                        if (pf4.is_point_on_face(face_i, r))
                        {
                           // graph.points.push_back(r);
                        }

                        if(pf.is_point_on_face(face_i,r) &&
                          ( pf5.is_point_on_face(face_j,r)  ))
                        {
                            int v0 = pf4.get_point_or_add(verts.vertices[i].V);
                            int v1 = pf4.get_point_or_add(verts.vertices[i+1].V);
                            int w0 = pf5.get_point_or_add(verts.vertices[i].V);
                            int w1 = pf5.get_point_or_add(verts.vertices[i+1].V);

                           // graph.points.push_back(verts.vertices[i].V);
                           // graph.points.push_back(verts.vertices[i+1].V);
                            //graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));

                            if(pf4.find_edge(v0,v1)!=-1)
                            {

                               // graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));
                            }

                            int new_e;

                            if(pf4.find_edge(v0,v1)==-1)
                            {
                                new_e = pf4.get_edge_or_add(v0,v1,3);
                                pf4.faces[face_i].addEdge(new_e);
                            }
                            else
                            {
                                new_e = pf4.find_edge(v0,v1);
                                pf4.edges[new_e].topo_group=3;
                            }

                            if(pf5.find_edge(w0,w1)==-1)
                            {
                                new_e = pf5.get_edge_or_add(w0,w1,3);
                                pf5.faces[face_j].addEdge(new_e);
                            }
                            else
                            {
                                new_e = pf5.find_edge(w0,w1);
                                pf5.edges[new_e].topo_group=3;
                                pf5.faces[face_j].addEdge(new_e);
                            }

                        }
                    }
                }
            }
        }

#ifdef BVH_OPTIMIZE
        for (BVH_intersection_struct hit : faces_faces)
        {
            u16 face_i = hit.X;
            u16 face_j = hit.Y;
#else
     for(int face_i=0;face_i<pf.faces.size();face_i++)
        for(int face_j=0;face_j<pf2.faces.size();face_j++)
        {
#endif
            poly_face& f=pf.faces[face_i];
            poly_face& f2=pf2.faces[face_j];

            core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
            core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);

           if(is_coplanar_point(f_plane,pf5.faces[face_j].m_center) && is_parallel_normal(pf4.faces[face_i].m_normal,pf2.faces[face_j].m_normal))
            {
                for(int e_i: pf5.faces[face_j].edges)
                {
                   core::vector3df v0 = pf5.getVertex(e_i,0).V;
                   core::vector3df v1 = pf5.getVertex(e_i,1).V;
                   core::vector3df r = v0 + (v1 - v0)*0.5;
                   int v_i0 = pf4.find_point(v0);
                   int v_i1 = pf4.find_point(v1);

                   if( v_i0 != -1 && v_i1 !=-1 && pf.is_point_on_face(face_i,r))
                        {
                            int new_e = pf4.get_edge_or_add(v_i0,v_i1,3);
                            pf4.edges[new_e].topo_group=3;
                            pf4.faces[face_i].addEdge(new_e);
                        }
                    //graph.lines.push_back(core::line3df(v0,v1));
                }
                for(int e_i: pf4.faces[face_i].edges)
                {
                    core::vector3df v0 = pf4.getVertex(e_i,0).V;
                    core::vector3df v1 = pf4.getVertex(e_i,1).V;
                    core::vector3df r = v0 + (v1 - v0)*0.5;
                    int v_i0 = pf5.find_point(v0);
                    int v_i1 = pf5.find_point(v1);

                    if( v_i0 != -1 && v_i1 !=-1 && pf2.is_point_on_face(face_j,r))
                    {
                        int new_e = pf5.get_edge_or_add(v_i0,v_i1,3);
                        pf5.edges[new_e].topo_group=3;
                        pf5.faces[face_j].addEdge(new_e);
                    }
                }
            }
        }
}*/

/*
inline void initial_topology_inner_loop(polyfold& pf4, polyfold& pf5, const core::plane3df& f2_plane, u16 e_i, u16 face_j)
{
    static LineHolder nograph;
    core::vector3df ipoint;
    core::vector3df v0 = pf4.getVertex(e_i, 0).V;
    core::vector3df v1 = pf4.getVertex(e_i, 1).V;
    if (line_intersects_plane(f2_plane, v0, v1, ipoint) && pf4.edges[e_i].topo_group != -1
        && pf5.is_point_on_face(face_j, ipoint))
    {
        //int new_v = pf4.get_point_or_add(ipoint);
        int new_v = pf4.find_point(ipoint);

        if (new_v == pf4.edges[e_i].v0)
        {
            v0 = pf4.getVertex(e_i, 0).V;
            v1 = pf4.getVertex(e_i, 1).V;
        }
        else
        {
            v1 = pf4.getVertex(e_i, 0).V;
            v0 = pf4.getVertex(e_i, 1).V;
        }

        if (pf4.edges[e_i].topo_group == 2)
        {
            if (pf5.is_point_on_face(face_j, ipoint))
            {
                int RES = pf5.classify_point(face_j, v0, v1, nograph);

                if (RES == TOP_FRONT)
                    pf4.edges[e_i].topo_group = 0;
                else if (RES == TOP_BEHIND)
                    pf4.edges[e_i].topo_group = 1;
            }
        }
        //new_v = pf5.get_point_or_add(ipoint);
        //new_v = pf5.find_point(ipoint);
        //pf5.faces[face_j].addVertex(new_v);
    }
}
*/
/*
void do_initial_topology(polyfold& pf4, polyfold& pf5, const std::vector<BVH_intersection_struct>& edges_faces, LineHolder& graph)
{

#ifdef BVH_OPTIMIZE
    for (const BVH_intersection_struct& hit : edges_faces)
    {
        u16 e_i = hit.X;
        u16 face_j = hit.Y;
#else
    for(int face_j=0;face_j<pf5.faces.size();face_j++)
    {
#endif
        poly_face& f2 = pf5.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);
        core::vector3df ipoint;

#ifndef BVH_OPTIMIZE
        for(int e_i=0; e_i<pf4.edges.size(); e_i++)
#endif
        {
            core::vector3df v0 = pf4.getVertex(e_i,0).V;
            core::vector3df v1 = pf4.getVertex(e_i,1).V;
            if(line_intersects_plane(f2_plane,v0,v1,ipoint) && pf4.edges[e_i].topo_group != -1
                && pf5.is_point_on_face(face_j,ipoint))
            {
                //int new_v = pf4.get_point_or_add(ipoint);
                int new_v = pf4.find_point(ipoint);

                if(new_v == pf4.edges[e_i].v0)
                {
                    v0 = pf4.getVertex(e_i,0).V;
                    v1 = pf4.getVertex(e_i,1).V;
                }
                else
                {
                    v1 = pf4.getVertex(e_i,0).V;
                    v0 = pf4.getVertex(e_i,1).V;
                }

                if(pf4.edges[e_i].topo_group ==2)
                {
                    if(pf5.is_point_on_face(face_j,ipoint))
                    {
                        //graph.points.push_back(v1);
                        int RES = pf5.classify_point(face_j,v0,v1,graph);

                        if(RES == TOP_FRONT)
                            pf4.edges[e_i].topo_group=0;
                        else if(RES == TOP_BEHIND)
                            pf4.edges[e_i].topo_group=1;
                    }
                }

                //new_v = pf5.get_point_or_add(ipoint);
                //new_v = pf5.find_point(ipoint);
                //pf5.faces[face_j].addVertex(new_v);
            }
        }
    }
}
*/


/*
void polyfold::classify_edge(int e_0)
{
    edges[e_0].conv = EDGE_UNDEF;
    std::vector<u16> nfaces;

#ifdef OPTIMIZE_CLASSIFY_EDGES
    std::vector<u16> hits;

    aabb_struct aabb;
    edges[e_0].grow(&aabb, vertices.data());

    faces_BVH.intersect(aabb, hits);

    for (u16 f_i : hits)
    {
        for (int e_i : this->faces[f_i].edges)
        {
            if (e_i == e_0)
                nfaces.push_back(f_i);
        }
    }
#else
    for (int f_i = 0; f_i < this->faces.size(); f_i++)
    {
        for (int e_i : this->faces[f_i].edges)
        {
            if (e_i == e_0)
                nfaces.push_back(f_i);
        }
    }
#endif

    if (nfaces.size() == 1)
    {
        this->edges[e_0].conv = EDGE_SIMPLE;
        return;
    }
    else if (nfaces.size() > 2)
    {
        this->edges[e_0].conv = EDGE_COMPLEX;
        return;
    }
    else if (nfaces.size() == 0)
        return;

    std::vector<core::vector3df> face_vecs;

    core::vector3df v1 = this->getVertex(e_0, 0).V - this->getVertex(e_0, 1).V;

    for (int f_i : nfaces)
    {
        core::vector3df r = v1.crossProduct(this->faces[f_i].m_normal);
        r.normalize();

        if (this->is_point_on_face(f_i, this->getVertex(e_0, 1).V + v1 * 0.5 + r))
            face_vecs.push_back(r);
        else
            face_vecs.push_back(-r);
    }

    core::matrix4 R;
    v1.normalize();
    R.buildRotateFromTo(v1, core::vector3df(0, 1, 0));

    core::vector3df m1;
    core::vector3df m2;
    core::vector3df r1 = this->getVertex(e_0, 1).V + v1 * 0.5 + face_vecs[0];
    core::vector3df r2 = this->getVertex(e_0, 1).V + v1 * 0.5 + face_vecs[1];

    LineHolder nograph;

    R.rotateVect(m1, this->faces[nfaces[0]].m_normal);
    R.rotateVect(m2, this->faces[nfaces[1]].m_normal);

    R.rotateVect(r1);
    R.rotateVect(r2);
    m1.normalize();
    m2.normalize();

    core::line2df line1 = core::line2df(r1.X, r1.Z, r1.X + m1.X, r1.Z + m1.Z);
    core::line2df line2 = core::line2df(r2.X, r2.Z, r2.X + m2.X, r2.Z + m2.Z);

    core::vector2df ipoint;

    if (line1.intersectWith(line2, ipoint, false))
    {
        if (core::vector2df(m1.X, m1.Z).dotProduct(ipoint - core::vector2df(r1.X, r1.Z)) < 0
            && core::vector2df(m2.X, m2.Z).dotProduct(ipoint - core::vector2df(r2.X, r2.Z)) < 0)
        {
            this->edges[e_0].conv = EDGE_CONVEX;
        }
        else
        {
            this->edges[e_0].conv = EDGE_CONCAVE;
        }
    }
}

void polyfold::classify_edges(LineHolder& graph)
{
    for (int e_0 = 0; e_0 < this->edges.size(); e_0++)
    {
        classify_edge(e_0);
    }
}
*/


/*
template<>
int do_intersections_and_bisect<true>(polyfold& pf4, polyfold& pf5, poly_intersection_info& intersect_info, LineHolder& graph)
{
    int n_intersections = 0;

    for (poly_edge& edge : pf4.edges)
    {
        edge.topo_group = 2;
    }

    for (poly_edge& edge : pf5.edges)
    {
        edge.topo_group = 2;
    }

    std::vector<core::plane3df> pf5_planes;
    pf5_planes.resize(pf5.faces.size());

    for (int face_j = 0; face_j < pf5.faces.size(); face_j++)
    {
        const poly_face& f = pf5.faces[face_j];
        pf5_planes[face_j] = core::plane3df(f.m_center, f.m_normal);
    }

    std::vector<u16> hits;
    core::vector3df ipoint;

    //intersections and bisect - pf4
    for (int e_i = 0; e_i < pf4.edges.size(); e_i++)
    {
        hits.clear();

        aabb_struct aabb;
        pf4.edges[e_i].grow(&aabb, pf4.vertices.data());
        pf5.faces_BVH.intersect(aabb, hits);

        for (u16 face_j : hits)
        {

            core::vector3df v0 = pf4.getVertex(e_i, 0).V;
            core::vector3df v1 = pf4.getVertex(e_i, 1).V;

            if (line_intersects_plane(pf5_planes[face_j], v0, v1, ipoint) && pf4.edges[e_i].topo_group != -1
                && pf5.is_point_on_face(face_j, ipoint))
            {
                int new_v = pf4.get_point_or_add(ipoint);
                if (pf4.bisect_edge(e_i, new_v, 2, 2))
                {

                }
                n_intersections++;
            }
        }
    }

    //intersections and bisect - pf5

    std::vector<core::plane3df> pf4_planes;
    pf4_planes.resize(pf4.faces.size());

    for (int face_j = 0; face_j < pf4.faces.size(); face_j++)
    {
        const poly_face& f = pf4.faces[face_j];
        pf4_planes[face_j] = core::plane3df(f.m_center, f.m_normal);
    }

    for (u16 e_i = 0; e_i < pf5.edges.size(); e_i++)
    {
        hits.clear();

        aabb_struct aabb;

        pf5.edges[e_i].grow(&aabb, pf5.vertices.data());
        pf4.faces_BVH.intersect(aabb, hits);

        for (u16 face_j : hits)
        {

            core::vector3df v0 = pf5.getVertex(e_i, 0).V;
            core::vector3df v1 = pf5.getVertex(e_i, 1).V;
            if (line_intersects_plane(pf4_planes[face_j], v0, v1, ipoint) && pf5.edges[e_i].topo_group != -1
                && pf.is_point_on_face(face_j, ipoint))
            {
                int new_v = pf5.get_point_or_add(ipoint);

                if (pf5.bisect_edge(e_i, new_v, 2, 2))
                {

                }
                n_intersections++;
            }

        }
    }

    for (int v_i = 0; v_i < pf5.vertices.size(); v_i++)
    {
        if (pf4.find_point(pf5.vertices[v_i].V) == -1)
        {
            int new_v = -1;
            for (int f_i = 0; f_i < pf4.faces.size(); f_i++)
            {
                if (pf4.point_is_coplanar(f_i, pf5.vertices[v_i].V) &&
                    pf4.is_point_on_face(f_i, pf5.vertices[v_i].V))
                {
                    new_v = pf4.get_point_or_add(pf5.vertices[v_i].V);
                    pf4.faces[f_i].addVertex(new_v);
                }
            }
            if (new_v != -1)
                for (int e_i = 0; e_i < pf4.edges.size(); e_i++)
                {
                    if (pf4.point_is_on_edge(e_i, pf5.vertices[v_i].V))
                    {
                        pf4.bisect_edge(e_i, new_v, 2, 2);
                        n_intersections++;
                    }
                }
        }
    }

    for (int v_i = 0; v_i < pf4.vertices.size(); v_i++)
    {
        if (pf5.find_point(pf4.vertices[v_i].V) == -1)
        {
            int new_v = -1;
            for (int f_i = 0; f_i < pf5.faces.size(); f_i++)
            {
                if (pf5.point_is_coplanar(f_i, pf4.vertices[v_i].V) &&
                    pf5.is_point_on_face(f_i, pf4.vertices[v_i].V))
                {
                    new_v = pf5.get_point_or_add(pf4.vertices[v_i].V);
                    pf5.faces[f_i].addVertex(new_v);
                }
            }

            if (new_v != -1)
                for (int e_i = 0; e_i < pf5.edges.size(); e_i++)
                {
                    if (pf5.point_is_on_edge(e_i, pf4.vertices[v_i].V))
                    {
                        pf5.bisect_edge(e_i, new_v, 2, 2);
                        n_intersections++;
                    }
                }
        }
    }

    return n_intersections;
}*/

/*
template<>
int do_intersections_and_bisect<true>(polyfold& pf, polyfold& pf2, polyfold& pf4, polyfold& pf5, poly_intersection_info& intersect_info, LineHolder& graph)
{
    int n_intersections = 0;

    for (poly_edge& edge : pf4.edges)
    {
        edge.topo_group = 2;
    }

    for (poly_edge& edge : pf5.edges)
    {
        edge.topo_group = 2;
    }

    //intersections and bisect - pf4

    for (int i = 0; i < intersect_info.edges_faces.size(); i++)
    {
        u16 e_i = intersect_info.edges_faces[i].X;
        u16 face_j = intersect_info.edges_faces[i].Y;

        poly_face& f2 = pf5.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f2.m_center, f2.m_normal);
        core::vector3df ipoint;

        core::vector3df v0 = pf4.getVertex(e_i, 0).V;
        core::vector3df v1 = pf4.getVertex(e_i, 1).V;

        if (line_intersects_plane(f2_plane, v0, v1, ipoint) && pf4.edges[e_i].topo_group != -1
            && pf5.is_point_on_face(face_j, ipoint))
        {
            int new_v = pf4.get_point_or_add(ipoint);
            if (pf4.bisect_edge(e_i, new_v, 2, 2))
            {
                for (int j = 0; j < intersect_info.edges_faces.size(); j++)
                {
                    if (intersect_info.edges_faces[j].X == e_i)
                    {
                        intersect_info.edges_faces[j].X = static_cast<u16>(pf4.edges.size() - 1);
                        intersect_info.edges_faces.push_back(BVH_intersection_struct{ static_cast<u16>(pf4.edges.size() - 2),intersect_info.edges_faces[j].Y });
                    }
                }
            }
            n_intersections++;
        }
    }

    //intersections and bisect - pf5

    for (int i = 0; i < intersect_info.faces_edges.size(); i++)
    {
        u16 face_j = intersect_info.faces_edges[i].X;
        u16 e_i = intersect_info.faces_edges[i].Y;

        poly_face& f = pf.faces[face_j];
        core::plane3df f_plane = core::plane3df(f.m_center, f.m_normal);
        core::vector3df ipoint;

        core::vector3df v0 = pf5.getVertex(e_i, 0).V;
        core::vector3df v1 = pf5.getVertex(e_i, 1).V;

        if (line_intersects_plane(f_plane, v0, v1, ipoint) && pf5.edges[e_i].topo_group != -1
            && pf.is_point_on_face(face_j, ipoint))
        {
            //    graph.lines.push_back(core::line3df(pf5.getVertex(e_i, 0).V, pf5.getVertex(e_i, 1).V));
            int new_v = pf5.get_point_or_add(ipoint);
            if (pf5.bisect_edge(e_i, new_v, 2, 2))
            {
                for (int j = 0; j < intersect_info.faces_edges.size(); j++)
                {
                    if (intersect_info.faces_edges[j].Y == e_i)
                    {
                        intersect_info.faces_edges[j].Y = static_cast<u16>(pf5.edges.size() - 1);
                        intersect_info.faces_edges.push_back(BVH_intersection_struct{ intersect_info.faces_edges[j].X,static_cast<u16>(pf5.edges.size() - 2) });
                    }
                }
            }
            n_intersections++;
        }

    }

    for (int v_i = 0; v_i < pf5.vertices.size(); v_i++)
    {
        if (pf4.find_point(pf5.vertices[v_i].V) == -1)
        {
            int new_v = -1;
            for (int f_i = 0; f_i < pf4.faces.size(); f_i++)
            {
                if (pf4.point_is_coplanar(f_i, pf5.vertices[v_i].V) &&
                    pf4.is_point_on_face(f_i, pf5.vertices[v_i].V))
                {
                    new_v = pf4.get_point_or_add(pf5.vertices[v_i].V);
                    pf4.faces[f_i].addVertex(new_v);
                }
            }
            if (new_v != -1)
                for (int e_i = 0; e_i < pf4.edges.size(); e_i++)
                {
                    if (pf4.point_is_on_edge(e_i, pf5.vertices[v_i].V))
                    {
                        pf4.bisect_edge(e_i, new_v, 2, 2);
                        n_intersections++;
                    }
                }
        }
    }

    for (int v_i = 0; v_i < pf4.vertices.size(); v_i++)
    {
        if (pf5.find_point(pf4.vertices[v_i].V) == -1)
        {
            int new_v = -1;
            for (int f_i = 0; f_i < pf5.faces.size(); f_i++)
            {
                if (pf5.point_is_coplanar(f_i, pf4.vertices[v_i].V) &&
                    pf5.is_point_on_face(f_i, pf4.vertices[v_i].V))
                {
                    new_v = pf5.get_point_or_add(pf4.vertices[v_i].V);
                    pf5.faces[f_i].addVertex(new_v);
                }
            }

            if (new_v != -1)
                for (int e_i = 0; e_i < pf5.edges.size(); e_i++)
                {
                    if (pf5.point_is_on_edge(e_i, pf4.vertices[v_i].V))
                    {
                        pf5.bisect_edge(e_i, new_v, 2, 2);
                        n_intersections++;
                    }
                }
        }
    }

    return n_intersections;
}
*/

/*
template<>
int do_intersections_and_bisect<false>(polyfold& pf, polyfold& pf2, polyfold& pf4, polyfold& pf5, poly_intersection_info& intersect_info, LineHolder& graph)
{
    int n_intersections = 0;

    for (poly_edge& edge : pf4.edges)
    {
        edge.topo_group = 2;
    }

    for (poly_edge& edge : pf5.edges)
    {
        edge.topo_group = 2;
    }

    //intersections and bisect - pf4

    for (int face_j = 0; face_j < pf5.faces.size(); face_j++)
    {
        poly_face& f2 = pf5.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f2.m_center, f2.m_normal);
        core::vector3df ipoint;
        for (int e_i = 0; e_i < pf4.edges.size(); e_i++)
        {
            core::vector3df v0 = pf4.getVertex(e_i, 0).V;
            core::vector3df v1 = pf4.getVertex(e_i, 1).V;

            if (line_intersects_plane(f2_plane, v0, v1, ipoint) && pf4.edges[e_i].topo_group != -1
                && pf5.is_point_on_face(face_j, ipoint))
            {
                int new_v = pf4.get_point_or_add(ipoint);
                if (pf4.bisect_edge(e_i, new_v, 2, 2))
                {

                }
                n_intersections++;
            }
        }
    }

    //intersections and bisect - pf5

    for (u16 face_j = 0; face_j < pf.faces.size(); face_j++)
    {
        poly_face& f = pf.faces[face_j];
        core::plane3df f_plane = core::plane3df(f.m_center, f.m_normal);
        core::vector3df ipoint;

        for (u16 e_i = 0; e_i < pf5.edges.size(); e_i++)
        {

            core::vector3df v0 = pf5.getVertex(e_i, 0).V;
            core::vector3df v1 = pf5.getVertex(e_i, 1).V;
            if (line_intersects_plane(f_plane, v0, v1, ipoint) && pf5.edges[e_i].topo_group != -1
                && pf.is_point_on_face(face_j, ipoint))
            {
                //    graph.lines.push_back(core::line3df(pf5.getVertex(e_i, 0).V, pf5.getVertex(e_i, 1).V));
                int new_v = pf5.get_point_or_add(ipoint);
                if (pf5.bisect_edge(e_i, new_v, 2, 2))
                {

                }
                n_intersections++;
            }
        }
    }

    for (int v_i = 0; v_i < pf5.vertices.size(); v_i++)
    {
        if (pf4.find_point(pf5.vertices[v_i].V) == -1)
        {
            int new_v = -1;
            for (int f_i = 0; f_i < pf4.faces.size(); f_i++)
            {
                if (pf4.point_is_coplanar(f_i, pf5.vertices[v_i].V) &&
                    pf4.is_point_on_face(f_i, pf5.vertices[v_i].V))
                {
                    new_v = pf4.get_point_or_add(pf5.vertices[v_i].V);
                    pf4.faces[f_i].addVertex(new_v);
                }
            }
            if (new_v != -1)
                for (int e_i = 0; e_i < pf4.edges.size(); e_i++)
                {
                    if (pf4.point_is_on_edge(e_i, pf5.vertices[v_i].V))
                    {
                        pf4.bisect_edge(e_i, new_v, 2, 2);
                        n_intersections++;
                    }
                }
        }
    }

    for (int v_i = 0; v_i < pf4.vertices.size(); v_i++)
    {
        if (pf5.find_point(pf4.vertices[v_i].V) == -1)
        {
            int new_v = -1;
            for (int f_i = 0; f_i < pf5.faces.size(); f_i++)
            {
                if (pf5.point_is_coplanar(f_i, pf4.vertices[v_i].V) &&
                    pf5.is_point_on_face(f_i, pf4.vertices[v_i].V))
                {
                    new_v = pf5.get_point_or_add(pf4.vertices[v_i].V);
                    pf5.faces[f_i].addVertex(new_v);
                }
            }

            if (new_v != -1)
                for (int e_i = 0; e_i < pf5.edges.size(); e_i++)
                {
                    if (pf5.point_is_on_edge(e_i, pf4.vertices[v_i].V))
                    {
                        pf5.bisect_edge(e_i, new_v, 2, 2);
                        n_intersections++;
                    }
                }
        }
    }

    return n_intersections;
}

*/

/*
* //test code from void polyfold::finalize_clipped_poly(const polyfold& pf, LineHolder& graph)
    for (int e_i = 0; e_i < pf.edges.size(); e_i++)
    {
        bool tb = false;
        for (int i = 0; i < pf.edges_BVH.node_count; i++)
        {
            const BVH_node& node = pf.edges_BVH.nodes[i];
            if (node.isLeafNode())
            {
                if(pf.edges_BVH.indices[node.first_prim] == e_i)
                    tb = true;
               // for (int j = node.first_prim; j < node.first_prim + node.n_prims; j++)
               // {
               //     if(pf.edges_BVH.indices[j] == e_i)
               //         tb = true;
               // }
            }
        }
        if (tb == false)
        {
            std::cout << "could not find edge " << e_i << " ";
            std::cout << pf.edges[e_i].topo_group << "\n";
            for (int i = 0; i < pf.edges_BVH.node_count; i++)
            {
                const BVH_node& node = pf.edges_BVH.nodes[i];
                if (node.isLeafNode())
                {
                    for (int j = node.first_prim; j < node.first_prim + node.n_prims; j++)
                    {
                        if (pf.edges_BVH.indices[j] == e_i)
                        {
                            for (int jj = node.first_prim; jj < node.first_prim + node.n_prims; jj++)
                            {
                                int v_i = pf.edges_BVH.indices[jj];
                                std::cout << pf.edges[v_i].topo_group << ",";
                                //vector3df pos = pf.edges[v_i].position(pf.vertices.data());
                                //std::cout << pos.X << "," << pos.Y << "," << pos.Z << " / ";
                            }
                            std::cout << "\n";
                        }
                    }
                }
            }
        }
    }*/