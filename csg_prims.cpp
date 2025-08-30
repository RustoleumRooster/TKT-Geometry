#include <iostream>
#include "csg_classes.h"
#include "GeometryStack.h"
#include <algorithm>
#include "soa.h"

using namespace irr;

#define PRINTV(x) x.X <<","<<x.Y<<","<<x.Z<<" "

#define pi 3.141592653

core::vector3df get_center(polyfold pf)
{
    core::vector3df ret(0,0,0);
    for(int i=0;i<pf.vertices.size();i++)
        ret+=pf.vertices[i].V;
    ret/=pf.vertices.size();
    return ret;
}

f32 get_angle_phi_for_face(vector3df point, vector3df vec1, vector3df iY, polyfold* pf, poly_face* f)
{
    vector3df center = vector3df(0, 0, 0);

    for (int v_i : f->vertices)
    {
        center += pf->vertices[v_i].V;
    }
    center /= f->vertices.size();

    core::vector3df r = center - point;
    r.normalize();

    vector2df k;

    k.X = r.dotProduct(vec1);
    k.Y = -r.dotProduct(iY);

    f32 az = k.getAngleTrig();
    return az;
}

void add_face_vertices_from_edges(polyfold& pf, poly_face& f)
{
    for (int e_i : f.edges)
    {
        f.addVertex(pf.edges[e_i].v0);
        f.addVertex(pf.edges[e_i].v1);
    }
}

void do_primitive_calculations(polyfold& pf, bool calc_normals = true)
{
    for(int f_i=0;f_i<pf.faces.size();f_i++)
    {
        pf.faces[f_i].face_id = f_i;

        for(int e_i:pf.faces[f_i].edges)
        {
            pf.faces[f_i].addVertex(pf.edges[e_i].v0);
            pf.faces[f_i].addVertex(pf.edges[e_i].v1);
        }

        if(calc_normals)
            pf.calc_normal(f_i);
    }

    f32 xsum=0;
    f32 ysum=0;
    f32 zsum=0;

    for(int i=0; i < pf.vertices.size(); i++)
    {
        xsum+=pf.vertices[i].V.X;
        ysum+=pf.vertices[i].V.Y;
        zsum+=pf.vertices[i].V.Z;
    }

    xsum/=pf.vertices.size();
    ysum/=pf.vertices.size();
    zsum/=pf.vertices.size();

    core::vector3df pf_center(xsum,ysum,zsum);

    if (calc_normals)
    {
        for (int f_i = 0; f_i < pf.faces.size(); f_i++)
        {

            core::vector3df r = pf.faces[f_i].m_center - pf_center;
            f32 dp = pf.faces[f_i].m_normal.dotProduct(r);
            if (dp < 0)
            {
                pf.faces[f_i].flip_normal();
            }
        }
    }

    pf.recalc_bbox_and_loops();

    pf.topology = TOP_CONVEX;
}

geo_element make_poly_cube(int height,int length,int width)
{
    geo_element geo;
    polyfold& pf = geo.brush;

    pf.vertices.push_back(poly_vert(0,0,0)); //0
    pf.vertices.push_back(poly_vert(length,0,0));
    pf.vertices.push_back(poly_vert(0,0,width));
    pf.vertices.push_back(poly_vert(length,0,width));

    pf.vertices.push_back(poly_vert(0,height,0)); //4
    pf.vertices.push_back(poly_vert(length,height,0));
    pf.vertices.push_back(poly_vert(0,height,width));
    pf.vertices.push_back(poly_vert(length,height,width));

    pf.edges.push_back(poly_edge(0,1));//0
    pf.edges.push_back(poly_edge(2,3)); //2,4,6,10
    pf.edges.push_back(poly_edge(0,2)); //3,5,7,11
    pf.edges.push_back(poly_edge(1,3));

    pf.edges.push_back(poly_edge(0,4));//4
    pf.edges.push_back(poly_edge(1,5));
    pf.edges.push_back(poly_edge(2,6));
    pf.edges.push_back(poly_edge(3,7));

    pf.edges.push_back(poly_edge(4,5));//8
    pf.edges.push_back(poly_edge(6,7));
    pf.edges.push_back(poly_edge(4,6));
    pf.edges.push_back(poly_edge(5,7));

    poly_face f;

    f.clear();
    f.addEdge(0);
    f.addEdge(1);
    f.addEdge(2);
    f.addEdge(3);
    pf.faces.push_back(f);
    pf.calc_normal(0);

    f.clear();
    f.addEdge(0);
    f.addEdge(8);
    f.addEdge(4);
    f.addEdge(5);
    pf.faces.push_back(f);

    f.clear();
    f.addEdge(1);
    f.addEdge(9);
    f.addEdge(6);
    f.addEdge(7);
    pf.faces.push_back(f);

    f.clear();
    f.addEdge(2);
    f.addEdge(4);
    f.addEdge(6);
    f.addEdge(10);
    pf.faces.push_back(f);

    f.clear();
    f.addEdge(3);
    f.addEdge(5);
    f.addEdge(7);
    f.addEdge(11);
    pf.faces.push_back(f);

    f.clear();
    f.addEdge(8);
    f.addEdge(9);
    f.addEdge(10);
    f.addEdge(11);
    pf.faces.push_back(f);

    do_primitive_calculations(pf);

    surface_group sg;

    sg.type = SURFACE_GROUP_STANDARD;
    sg.point = get_center(pf);
    sg.vec = core::vector3df(0,1,0);
    sg.vec1 = core::vector3df(0,0,1);

    pf.surface_groups.push_back(sg);

    for(poly_face& face: pf.faces)
    {
        face.surface_group=0;
        face.uv_origin=pf.vertices[face.vertices[0]].V;
    }

    geo.surfaces.resize(pf.faces.size());

    for (int i = 0; i < pf.faces.size(); i++)
    {
        geo.surfaces[i].my_faces.push_back(i);
        pf.faces[i].surface_no = i;
    }

    return geo;
}

geo_element make_poly_plane(int length, int width)
{
    geo_element geo;
    polyfold& pf = geo.brush;

    pf.vertices.push_back(poly_vert(0,0,0));
    pf.vertices.push_back(poly_vert(length,0,0));
    pf.vertices.push_back(poly_vert(0,0,width));
    pf.vertices.push_back(poly_vert(length,0,width));

    pf.edges.push_back(poly_edge(0,1));
    pf.edges.push_back(poly_edge(2,3));
    pf.edges.push_back(poly_edge(0,2));
    pf.edges.push_back(poly_edge(1,3));

    poly_face f;

    f.addEdge(0);
    f.addEdge(1);
    f.addEdge(2);
    f.addEdge(3);

    pf.faces.push_back(f);

    for(int f_i=0;f_i<pf.faces.size();f_i++)
    {
        for(int e_i:pf.faces[f_i].edges)
        {
            pf.faces[f_i].addVertex(pf.edges[e_i].v0);
            pf.faces[f_i].addVertex(pf.edges[e_i].v1);
        }
        pf.calc_normal(f_i);
    }

    pf.recalc_bbox_and_loops();
    pf.topology = TOP_UNDEF;
    //====

    surface_group sg;

    sg.type = SURFACE_GROUP_STANDARD;
    sg.point = get_center(pf);
    sg.vec = core::vector3df(0,1,0);
    sg.vec1 = core::vector3df(0,0,1);

    pf.surface_groups.push_back(sg);

    pf.faces[0].surface_group=0;

    geo.surfaces.resize(pf.faces.size());

    for (int i = 0; i < pf.faces.size(); i++)
    {
        geo.surfaces[i].surface_type = SURFACE_GROUP_STANDARD;
        geo.surfaces[i].my_faces.push_back(i);
        pf.faces[i].surface_no = i;
    }

    return geo;
}

geo_element make_curve(int degrees_begin, int degrees_end, int inner_radius0, int outer_radius0, int height, int nSections)
{
    geo_element geo;
    polyfold& pf = geo.brush;

    f32 inner_radius = inner_radius0 / cos(pi / nSections);
    f32 outer_radius = outer_radius0 / cos(pi / nSections);

    f32 start_angle = pi * ((f32)degrees_begin / 180.0);
    f32 end_angle = pi * ((f32)degrees_end / 180.0);
    f32 theta_inc = (end_angle - start_angle) / nSections;

    vector<int> top_face, bottom_face, inner_curve, outer_curve, cap_begin, cap_end;
    cap_begin = vector<int>{ 0,1,2,3 };
    cap_end = vector<int>{ 4 * nSections , 4 * nSections + 1, 4 * nSections + 2, 4 * nSections + 3};

    canonical_brush top_brush, bottom_brush, inner_brush, outer_brush;
    top_brush.initialize(nSections);
    bottom_brush.initialize(nSections);
    inner_brush.initialize(nSections);
    outer_brush.initialize(nSections);

    pf.surface_groups.resize(5);

    geo.surfaces.resize(6);

    geo.surfaces[0].surface_type = SURFACE_GROUP_CANONICAL;
    geo.surfaces[1].surface_type = SURFACE_GROUP_CANONICAL;
    geo.surfaces[2].surface_type = SURFACE_GROUP_CYLINDER;
    geo.surfaces[3].surface_type = SURFACE_GROUP_CYLINDER;
    geo.surfaces[4].surface_type = SURFACE_GROUP_STANDARD;
    geo.surfaces[5].surface_type = SURFACE_GROUP_STANDARD;

    //int inner_len = 0;
    //int outer_len = 0;

    for (int i = 0; i < nSections + 1; i++)
    {
        f32 theta = start_angle + theta_inc * i;
        pf.vertices.push_back(poly_vert(cos(theta) * inner_radius, 0, sin(theta) * inner_radius));
        pf.vertices.push_back(poly_vert(cos(theta) * inner_radius, height, sin(theta) * inner_radius));

        pf.vertices.push_back(poly_vert(cos(theta) * outer_radius, 0, sin(theta) * outer_radius));
        pf.vertices.push_back(poly_vert(cos(theta) * outer_radius, height, sin(theta) * outer_radius));

        top_face.push_back(i * 4 + 1);
        top_face.push_back(i * 4 + 3);

        bottom_face.push_back(i * 4);
        bottom_face.push_back(i * 4 + 2);

        inner_curve.push_back(i * 4);
        inner_curve.push_back(i * 4 + 1);

        outer_curve.push_back(i * 4 + 2);
        outer_curve.push_back(i * 4 + 3);
    }

    //TOP
    //if(false)
    {
        pf.faces.push_back(poly_face{});
        int f_no = pf.faces.size() - 1;
        poly_face& f = pf.faces[f_no];

        int e0 = pf.get_edge_or_add(top_face[0], top_face[1], 0);
        int e1 = pf.get_edge_or_add(top_face[nSections * 2], top_face[nSections * 2 + 1], 0);

        f.addEdge(e0);
        f.addEdge(e1);

        for (int i = 0; i < nSections; i++)
        {
            e0 = pf.get_edge_or_add(top_face[2 * i], top_face[2 * (i + 1)], 0);
            e1 = pf.get_edge_or_add(top_face[2 * i + 1], top_face[2 * (i + 1) + 1], 0);

            f.addEdge(e0);
            f.addEdge(e1);

            top_brush.init_quad(i, &pf, f_no, top_face[2 * i + 1],
                top_face[2 * (i + 1) + 1],
                top_face[2 * (i + 1)],
                top_face[2 * i]);

           // inner_len += vector3df(pf.vertices[top_face[2 * i]].V - pf.vertices[top_face[2 * (i + 1)]].V).getLength();
           // outer_len += vector3df(pf.vertices[top_face[2 * i + 1]].V - pf.vertices[top_face[2 * (i + 1) + 1]].V).getLength();
        }
        //cout <<"len ="<< inner_len << " " << outer_len << "\n";

        f.surface_group = 0;
        f.surface_no = 0;
        geo.surfaces[0].my_faces.push_back(f_no);

        surface_group& sg = pf.surface_groups[0];
        sg.type = SURFACE_GROUP_CANONICAL;
        sg.point = vector3df{ 0,0,0 };
        sg.vec = core::vector3df(0, 1, 0);
        sg.vec1 = core::vector3df(0, 0, 1);
        sg.c_brush = top_brush;

        add_face_vertices_from_edges(pf, f);

        pf.calc_center(f_no);
        f.m_normal = vector3df{ 0,1,0 };
    }

    //BOTTOM
    //if(false)
    {
        pf.faces.push_back(poly_face{});
        int f_no = pf.faces.size() - 1;
        poly_face& f = pf.faces[f_no];

        int e0 = pf.get_edge_or_add(bottom_face[0], bottom_face[1], 0);
        int e1 = pf.get_edge_or_add(bottom_face[nSections * 2], bottom_face[nSections * 2 + 1], 0);

        f.addEdge(e0);
        f.addEdge(e1);

        for (int i = 0; i < nSections; i++)
        {
            e0 = pf.get_edge_or_add(bottom_face[2 * i], bottom_face[2 * (i + 1)], 0);
            e1 = pf.get_edge_or_add(bottom_face[2 * i + 1], bottom_face[2 * (i + 1) + 1], 0);

            f.addEdge(e0);
            f.addEdge(e1);

            bottom_brush.init_quad(i, &pf, f_no, bottom_face[2 * i + 1],
                bottom_face[2 * (i + 1) + 1],
                bottom_face[2 * (i + 1)],
                bottom_face[2 * i]);
        }

        f.surface_group = 1;
        f.surface_no = 1;
        geo.surfaces[1].my_faces.push_back(f_no);

        surface_group& sg = pf.surface_groups[1];
        sg.type = SURFACE_GROUP_CANONICAL;
        sg.point = vector3df{ 0,0,0 };
        sg.vec = core::vector3df(0, 1, 0);
        sg.vec1 = core::vector3df(0, 0, 1);
        sg.c_brush = bottom_brush;

        add_face_vertices_from_edges(pf, f);

        pf.calc_center(f_no);
        f.m_normal = vector3df{ 0,-1,0 };
    }

    //INNER
    //if(false)
    {
        for (int i = 0; i < nSections; i++)
        {
            pf.faces.push_back(poly_face{});
            int f_no = pf.faces.size() - 1;
            poly_face& f = pf.faces[f_no];

            int e0 = pf.get_edge_or_add(inner_curve[2 * i], inner_curve[2 * (i + 1)], 0);
            int e1 = pf.get_edge_or_add(inner_curve[2 * i + 1], inner_curve[2 * (i + 1) + 1], 0);
            int e2 = pf.get_edge_or_add(inner_curve[2 * i], inner_curve[2 * i + 1], 0);
            int e3 = pf.get_edge_or_add(inner_curve[2 * (i + 1)], inner_curve[2 * (i + 1) + 1], 0);

            f.addEdge(e0);
            f.addEdge(e1);
            f.addEdge(e2);
            f.addEdge(e3);

            inner_brush.init_quad(i, &pf, f_no, inner_curve[2 * i + 1],
                inner_curve[2 * (i + 1) + 1],
                inner_curve[2 * (i + 1)],
                inner_curve[2 * i]);

            f.surface_group = 2;
            f.surface_no = 2;
            geo.surfaces[2].my_faces.push_back(f_no);

            add_face_vertices_from_edges(pf, f);

            pf.calc_normal(f_no);
            if (f.m_normal.dotProduct(f.m_center) > 0)
                f.flip_normal();
        }

        surface_group& sg = pf.surface_groups[2];
        sg.type = SURFACE_GROUP_CYLINDER;
        sg.point = vector3df(0, 0, 0);
        sg.vec = core::vector3df(0, 1, 0);
        sg.vec1 = core::vector3df(0, 0, 1);
        sg.height = height;
        sg.radius = inner_radius;
        sg.c_brush = inner_brush;

        
    }

    //OUTER
    //if(false)
    {
        for (int i = 0; i < nSections; i++)
        {
            pf.faces.push_back(poly_face{});
            int f_no = pf.faces.size() - 1;
            poly_face& f = pf.faces[f_no];

            int e0 = pf.get_edge_or_add(outer_curve[2 * i], outer_curve[2 * (i + 1)], 0);
            int e1 = pf.get_edge_or_add(outer_curve[2 * i + 1], outer_curve[2 * (i + 1) + 1], 0);
            int e2 = pf.get_edge_or_add(outer_curve[2 * i], outer_curve[2 * i + 1], 0);
            int e3 = pf.get_edge_or_add(outer_curve[2 * (i + 1)], outer_curve[2 * (i + 1) + 1], 0);

            f.addEdge(e0);
            f.addEdge(e1);
            f.addEdge(e2);
            f.addEdge(e3);
            
            outer_brush.init_quad(i, &pf, f_no, outer_curve[2 * i + 1],
                outer_curve[2 * (i + 1) + 1],
                outer_curve[2 * (i + 1)],
                outer_curve[2 * i]);

            f.surface_group = 3;
            f.surface_no = 3;
            geo.surfaces[3].my_faces.push_back(f_no);

            add_face_vertices_from_edges(pf, f);

            pf.calc_normal(f_no);
            if (f.m_normal.dotProduct(f.m_center) < 0)
                f.flip_normal();
        }

        surface_group& sg = pf.surface_groups[3];
        sg.type = SURFACE_GROUP_CYLINDER;
        sg.point = vector3df(0, 0, 0);
        sg.vec = core::vector3df(0, 1, 0);
        sg.vec1 = core::vector3df(0, 0, 1);
        sg.height = height;
        sg.radius = outer_radius;
        sg.c_brush = outer_brush;
    }

    f32 ref_radius = inner_radius + (outer_radius - inner_radius) * 0.5;
    f32 ref_angle_0 = start_angle + theta_inc * 0.5;
    f32 ref_angle_1 = start_angle + theta_inc * (f32)(nSections - 0.5);

    vector3df ref_point_0 = vector3df(cos(ref_angle_0) * ref_radius, 0, sin(ref_angle_0) * ref_radius);
    vector3df ref_point_1 = vector3df(cos(ref_angle_1) * ref_radius, 0, sin(ref_angle_1) * ref_radius);
    
    //CAP 1
    //if (false)
    {
        surface_group& sg = pf.surface_groups[4];
        sg.type = SURFACE_GROUP_STANDARD;
        sg.point = vector3df{ 0,0,0 };
        sg.vec = core::vector3df(0, 1, 0);
        sg.vec1 = core::vector3df(0, 0, 1);

        pf.faces.push_back(poly_face{});
        int f_no = pf.faces.size() - 1;
        poly_face& f = pf.faces[f_no];

        int e0 = pf.get_edge_or_add(0, 1, 0);
        int e1 = pf.get_edge_or_add(1, 3, 0);
        int e2 = pf.get_edge_or_add(2, 3, 0);
        int e3 = pf.get_edge_or_add(2, 0, 0);

        f.addEdge(e0);
        f.addEdge(e1);
        f.addEdge(e2);
        f.addEdge(e3);
        f.surface_group = 4;
        f.surface_no = 4;
        geo.surfaces[4].my_faces.push_back(f_no);

        add_face_vertices_from_edges(pf, f);

        pf.calc_normal(f_no);
        if (f.m_normal.dotProduct(f.m_center - ref_point_0) < 0)
            f.flip_normal();
    }

    //CAP 2
    //if(false)
    {
        pf.faces.push_back(poly_face{});
        int f_no = pf.faces.size() - 1;
        poly_face& f = pf.faces[f_no];

        int e0 = pf.get_edge_or_add(cap_end[0], cap_end[1], 0);
        int e1 = pf.get_edge_or_add(cap_end[1], cap_end[3], 0);
        int e2 = pf.get_edge_or_add(cap_end[2], cap_end[3], 0);
        int e3 = pf.get_edge_or_add(cap_end[2], cap_end[0], 0);

        f.addEdge(e0);
        f.addEdge(e1);
        f.addEdge(e2);
        f.addEdge(e3);
        f.surface_group = 4;
        f.surface_no = 5;
        geo.surfaces[5].my_faces.push_back(f_no);

        add_face_vertices_from_edges(pf, f);

        pf.calc_normal(f_no);
        if (f.m_normal.dotProduct(f.m_center - ref_point_1) < 0)
            f.flip_normal();
    }

    pf.control_vertices.push_back(poly_vert(0, 0, 0));

    pf.recalc_bbox_and_loops();
    pf.topology = TOP_CONVEX;

    for (int f_i = 0; f_i < pf.faces.size(); f_i++)
    {
        pf.faces[f_i].face_id = f_i;
    }

    //do_primitive_calculations(pf,false);

    return geo;
}

geo_element make_cylinder(int height, int length, int faces, int radius_type)
{
    

    geo_element geo;
    polyfold& pf = geo.brush;

    f32 radius;

    if (radius_type == 1)
        radius = length;
    else radius = length / cos(pi / faces);

    for(int i=0;i<faces;i++)
    {
        f32 theta = (pi*2/faces)*i;
        pf.vertices.push_back(poly_vert(cos(theta)*radius,0,sin(theta)*radius));
        pf.vertices.push_back(poly_vert(cos(theta)*radius,height,sin(theta)*radius));
    }
    poly_face f;
    for(int i=0;i<faces-1;i++)
    {
        int a = pf.get_edge_or_add(i*2,(i+1)*2,0);
        f.addEdge(a);

    }
    int aa = pf.get_edge_or_add(0,(faces-1)*2,0);
    f.addEdge(aa);
    pf.faces.push_back(f);  //0

    f.clear();
    for(int i=0;i<faces-1;i++)
    {
        int a = pf.get_edge_or_add(i*2+1,(i+1)*2+1,0);
        f.addEdge(a);

    }
    aa = pf.get_edge_or_add(1,(faces-1)*2+1,0);
    f.addEdge(aa);
    pf.faces.push_back(f);  //1

    canonical_brush c_brush;
    c_brush.initialize(faces);

    for(int i=0;i<faces-1;i++)
    {
        //  i*2+1 ____  i*2+3
        //   |            |
        //   |            |
        //  i*2   ____  i*2+2

        c_brush.init_quad(i, &pf, i+2, i * 2 + 1, i * 2 + 3, i * 2 + 2, i * 2);
        //c_brush.dump_quad(&pf, i);

        f.clear();
        int a = pf.get_edge_or_add(i*2,i*2+1,0);
        int b = pf.get_edge_or_add(i*2+1,(i+1)*2+1,0);
        int c = pf.get_edge_or_add((i+1)*2,(i+1)*2+1,0);
        int d = pf.get_edge_or_add(i*2,(i+1)*2,0);

        f.addEdge(a);
        f.addEdge(b);
        f.addEdge(c);
        f.addEdge(d);
        f.column = i;
        pf.faces.push_back(f); //i+2
    }
   

    f.clear();
    int s = faces-1;
    int a = pf.get_edge_or_add(0*2+1,s*2+1,0);
    int b = pf.get_edge_or_add(0*2,s*2,0);
    int c = pf.get_edge_or_add(0*2,0*2+1,0);
    int d = pf.get_edge_or_add(s*2,s*2+1,0);

    c_brush.init_quad(s, &pf, faces+1, s * 2 + 1, 1, 0, s * 2);

    c_brush.layout_uvs(256, 64);

    for (int i = 0; i < c_brush.n_quads; i++)
    {
        /*
        cout <<
            c_brush.uvs[i * 4].V.X << "," << c_brush.uvs[i * 4].V.Y << "  " <<
            c_brush.uvs[i * 4 + 1].V.X << "," << c_brush.uvs[i * 4 + 1].V.Y << "  " <<
            c_brush.uvs[i * 4 + 2].V.X << "," << c_brush.uvs[i * 4 + 2].V.Y << "  " <<
            c_brush.uvs[i * 4 + 3].V.X << "," << c_brush.uvs[i * 4 + 3].V.Y << "\n";
            */

        /*cout <<
            c_brush.vertices[i*4] << " " <<
            c_brush.vertices[i*4+1] << " " <<
            c_brush.vertices[i*4+2] << " " <<
            c_brush.vertices[i*4+3] << "\n";
           */
    }

    f.addEdge(c);
    f.addEdge(a);
    f.addEdge(d);
    f.addEdge(b);
    f.column = faces - 1;
    pf.faces.push_back(f); //n_faces+1

    

    surface_group sg;

    sg.c_brush = c_brush;

    sg.type = SURFACE_GROUP_STANDARD;
    sg.point = get_center(pf);
    sg.vec = core::vector3df(0,1,0);
    sg.vec1 = core::vector3df(0,0,1);

    pf.surface_groups.push_back(sg);

    sg.type = SURFACE_GROUP_CYLINDER;
    sg.point = get_center(pf);
    sg.vec = core::vector3df(0,1,0);
    sg.vec1 = core::vector3df(0,0,1);
    sg.height = height;
    sg.radius = radius;
    //sg.n_columns = faces;

    pf.surface_groups.push_back(sg);

    do_primitive_calculations(pf);

    for(poly_face& face: pf.faces)
        face.surface_group=1;

    pf.faces[0].surface_group=0;
    pf.faces[1].surface_group=0;

    vector3df iY = sg.vec.crossProduct(sg.vec1);
    iY.normalize();

    {
        std::vector<poly_face*> face_ptrs;

        for (poly_face& f : pf.faces)
        {
            if (f.surface_group == 1 && f.vertices.size() > 0)
            {
                face_ptrs.push_back(&f);
            }
        }

        std::sort(face_ptrs.begin(), face_ptrs.end(),
            [&](poly_face* f_a, poly_face* f_b)
            {
                float phi_a = get_angle_phi_for_face(sg.point, sg.vec1, iY, &pf, f_a);
                float phi_b = get_angle_phi_for_face(sg.point, sg.vec1, iY, &pf, f_b);
                return phi_a < phi_b;
            });

        for (int j = 0; j < face_ptrs.size(); j++)
        {
            face_ptrs[j]->column = j;
        }
    }

    pf.control_vertices.push_back(poly_vert(0, 0, 0));

    geo.surfaces.resize(3);

    geo.surfaces[0].surface_type = SURFACE_GROUP_STANDARD;
    geo.surfaces[1].surface_type = SURFACE_GROUP_STANDARD;
    geo.surfaces[2].surface_type = SURFACE_GROUP_CYLINDER;

    geo.surfaces[0].my_faces.push_back(0);
    geo.surfaces[1].my_faces.push_back(1);

    pf.faces[0].surface_no = 0;
    pf.faces[1].surface_no = 1;

    for (int i = 2; i < pf.faces.size(); i++)
    {
        geo.surfaces[2].my_faces.push_back(i);
        pf.faces[i].surface_no = 2;
    }

    return geo;
}

geo_element make_cone(int height, int radius, int faces)
{

    geo_element geo;
    polyfold& pf = geo.brush;

    for(int i=0;i<faces;i++)
    {
        f32 theta = (pi*2/faces)*i;
        pf.vertices.push_back(poly_vert(cos(theta)*radius,0,sin(theta)*radius));
    }
    pf.vertices.push_back(poly_vert(0,height,0));

    poly_face f;

    for(int i=0;i<faces-1;i++)
    {
        int a = pf.get_edge_or_add(i,i+1,0);
        f.addEdge(a);

    }
    int aa = pf.get_edge_or_add(0,faces-1,0);
    f.addEdge(aa);
    pf.faces.push_back(f);

    for(int i=0;i<faces-1;i++)
    {
        f.clear();
        int a = pf.get_edge_or_add(i,faces,0);
        int b = pf.get_edge_or_add(i+1,faces,0);
        int c = pf.get_edge_or_add(i,i+1,0);

        f.addEdge(a);
        f.addEdge(b);
        f.addEdge(c);

        pf.faces.push_back(f);
    }

    f.clear();
    int s = faces-1;
    int a = pf.get_edge_or_add(0,faces,0);
    int b = pf.get_edge_or_add(s,faces,0);
    int c = pf.get_edge_or_add(0,s,0);

    f.addEdge(a);
    f.addEdge(b);
    f.addEdge(c);

    pf.faces.push_back(f);

    do_primitive_calculations(pf);

    surface_group sg;

    sg.type = SURFACE_GROUP_STANDARD;
    sg.point = get_center(pf);
    sg.vec = core::vector3df(0,1,0);
    sg.vec1 = core::vector3df(0,0,1);
/*
    pf.surface_groups.push_back(sg);

    sg.type = SURFACE_GROUP_CYLINDER;
    sg.point = get_center(pf);
    sg.vec = core::vector3df(0,1,0);
    sg.vec1 = core::vector3df(0,0,1);
*/
    pf.surface_groups.push_back(sg);

    for(poly_face& face: pf.faces)
        face.surface_group=0;

    //pf.faces[0].surface_group=0;

    return geo;
}

int num_rad_points(int radius, int faces, int zen_faces, int zen_j, bool simplify)
{

    int n_zen_points = zen_faces;
    int face_len0 = radius*pi*2 / faces;
    f32 zen_angle = -pi*0.5+(pi/n_zen_points)*zen_j;
    int face_len = cos(zen_angle)*radius*pi*2 / faces;
    if(simplify)
        return (face_len <= face_len0 /2) ? faces /2  : faces;
    else return faces;
}

core::vector3df get_sphere_point(int radius, int faces, int zen_faces, int zen_j, int az_i, bool simplify)
{

    int n_zen_points = zen_faces;
    int n_rad_points =  num_rad_points(radius, faces, zen_faces, zen_j, simplify);

    f32 zen_angle = -pi*0.5+(pi/n_zen_points)*zen_j;
    f32 az_angle = (pi*2/n_rad_points)*az_i;

    //std::cout <<zen_j <<"/"<<az_i<<"  "<< zen_angle << ", " << az_angle << "\n";

    f32 X = cos(az_angle)*cos(zen_angle)*radius;
    f32 Z = sin(az_angle)*cos(zen_angle)*radius;
    f32 Y = sin(zen_angle)*radius;

    return core::vector3df(X,Y,Z);
}



geo_element make_sphere(int radius, int rad_faces, int zen_faces, bool simplify)
{

    geo_element geo;
    polyfold& pf = geo.brush;

    int n_zen_points = zen_faces;
    int n_f=0;
    int n_t=0;

    for(int j=1;j<n_zen_points;j++)
    {
        int n_rad_points = num_rad_points(radius,rad_faces,zen_faces,j,simplify);
        int n_rad_points1 = num_rad_points(radius,rad_faces,zen_faces,j+1,simplify);
        int column_no = 0;

        for(int i=0;i<n_rad_points;i++)
        {
            if (j == 1)
            {
                int ii = (i + 1) % n_rad_points;
                int v0 = pf.get_point_or_add(get_sphere_point(radius, rad_faces, zen_faces, j, i, simplify));
                int v1 = pf.get_point_or_add(get_sphere_point(radius, rad_faces, zen_faces, j, ii, simplify));
                int z0 = pf.get_point_or_add(get_sphere_point(radius, rad_faces, zen_faces, 0, 0, simplify));

                int e0 = pf.get_edge_or_add(v0, z0, 0);
                int e1 = pf.get_edge_or_add(v1, z0, 0);
                int e2 = pf.get_edge_or_add(v0, v1, 0);

                poly_face f;

                f.addVertex(v0);
                f.addVertex(v1);
                f.addVertex(z0);

                f.addEdge(e0);
                f.addEdge(e1);
                f.addEdge(e2);

                f.row = j;
                f.column = column_no++;

                pf.faces.push_back(f);
                n_f += 1;
                n_t += 1;
            }
            if ( (j>=1 && j< n_zen_points-1) &&
                num_rad_points(radius,rad_faces,zen_faces,j,simplify) == num_rad_points(radius,rad_faces,zen_faces,j+1,simplify))
            {
                //std::cout << j << "<--****\n";
                int ii = (i+1)%n_rad_points;

                int v0 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j,i,simplify));
                int v1 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j,ii,simplify));
                int v2 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j+1,i,simplify));
                int v3 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j+1,ii,simplify));

                poly_face f;
                int e0 = pf.get_edge_or_add(v0,v2,0);
                int e2 = pf.get_edge_or_add(v2,v3,0);

                int z2 = pf.get_edge_or_add(v1,v3,0);
                int z0 = pf.get_edge_or_add(v0,v1,0);

                f.clear();

                f.addVertex(v0);
                f.addVertex(v1);
                f.addVertex(v2);
                f.addVertex(v3);

                f.addEdge(e0);
                f.addEdge(e2);
                f.addEdge(z0);
                f.addEdge(z2);

                f.row = j;
                f.column = column_no++;

                pf.faces.push_back(f);
                n_f+=2;
                n_t+=2;
            }

            if ((j>=1 && j< n_zen_points-1) &&
                num_rad_points(radius,rad_faces,zen_faces,j,simplify)/2 == num_rad_points(radius,rad_faces,zen_faces,j+1,simplify))
            {
                int ii = (i+1)%n_rad_points;
                int iii = (i+2)%n_rad_points;
                int k = i /2;
                int kk = (k+1)%(n_rad_points/2);

                int v0 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j,i,simplify));
                int v1 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j+1,k,simplify));
                int v2 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j,ii,simplify));
                int v3 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j+1,kk,simplify));
                int v4 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j,iii,simplify));
                //std::cout << j << "<----\n";
                poly_face f;
                if(i%2==0)
                {
                    int e0 = pf.get_edge_or_add(v2,v1,0);
                    int e1 = pf.get_edge_or_add(v2,v3,0);
                    int e2 = pf.get_edge_or_add(v1,v3,0);

                    f.addVertex(v1);
                    f.addVertex(v2);
                    f.addVertex(v3);

                    f.addEdge(e0);
                    f.addEdge(e1);
                    f.addEdge(e2);

                    f.row = j;
                    f.column = column_no++;

                    pf.faces.push_back(f);
                    n_f+=1;
                    n_t+=1;

                }
                else
                {
                    int e0 = pf.get_edge_or_add(v0,v3,0);
                    int e1 = pf.get_edge_or_add(v2,v3,0);
                    int e2 = pf.get_edge_or_add(v0,v2,0);

                    f.addVertex(v0);
                    f.addVertex(v2);
                    f.addVertex(v3);

                    f.addEdge(e0);
                    f.addEdge(e1);
                    f.addEdge(e2);

                    f.row = j;
                    f.column = column_no++;

                    pf.faces.push_back(f);
                    f.clear();

                    int e3 = pf.get_edge_or_add(v4,v3,0);
                    int e4 = pf.get_edge_or_add(v2,v3,0);
                    int e5 = pf.get_edge_or_add(v4,v2,0);

                    f.addVertex(v2);
                    f.addVertex(v3);
                    f.addVertex(v4);

                    f.addEdge(e3);
                    f.addEdge(e4);
                    f.addEdge(e5);

                    f.row = j;
                    f.column = column_no++;

                    pf.faces.push_back(f);
                    n_f+=2;
                    n_t+=2;
                }
            }

            if ((j>=1 && j< n_zen_points-1) &&
                num_rad_points(radius,rad_faces,zen_faces,j,simplify)/2 == num_rad_points(radius,rad_faces,zen_faces,j-1,simplify))
            {
                int ii = (i+1)%n_rad_points;
                int iii = (i+2)%n_rad_points;
                int k = i /2;
                int kk = (k+1)%(n_rad_points/2);

                int v0 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j,i,simplify));
                int v1 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j-1,k,simplify));
                int v2 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j,ii,simplify));
                int v3 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j-1,kk,simplify));
                int v4 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j,iii,simplify));
               // std::cout << j << "<----\n";
                poly_face f;
                if(i%2==0)
                {
                    int e0 = pf.get_edge_or_add(v2,v1,0);
                    int e1 = pf.get_edge_or_add(v2,v3,0);
                    int e2 = pf.get_edge_or_add(v1,v3,0);

                    f.addVertex(v1);
                    f.addVertex(v2);
                    f.addVertex(v3);

                    f.addEdge(e0);
                    f.addEdge(e1);
                    f.addEdge(e2);

                    f.row = j;
                    f.column = column_no++;

                    pf.faces.push_back(f);
                    n_f+=1;
                    n_t+=1;
                }
                else
                {
                    int e0 = pf.get_edge_or_add(v0,v3,0);
                    int e1 = pf.get_edge_or_add(v2,v3,0);
                    int e2 = pf.get_edge_or_add(v0,v2,0);

                    f.addVertex(v0);
                    f.addVertex(v2);
                    f.addVertex(v3);

                    f.addEdge(e0);
                    f.addEdge(e1);
                    f.addEdge(e2);

                    f.row = j;
                    f.column = column_no++;

                    pf.faces.push_back(f);
                    f.clear();

                    int e3 = pf.get_edge_or_add(v4,v3,0);
                    int e4 = pf.get_edge_or_add(v2,v3,0);
                    int e5 = pf.get_edge_or_add(v4,v2,0);

                    f.addVertex(v2);
                    f.addVertex(v3);
                    f.addVertex(v4);

                    f.addEdge(e3);
                    f.addEdge(e4);
                    f.addEdge(e5);

                    f.row = j;
                    f.column = column_no++;

                    pf.faces.push_back(f);
                    n_f+=2;
                    n_t+=2;
                }
            }
            if (j==n_zen_points-1)
            {
                int ii = (i+1)%n_rad_points;
                int v0 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j,i,simplify));
                int v1 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j,ii,simplify));
                int z0 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,n_zen_points,0,simplify));

                int e0 = pf.get_edge_or_add(v0,z0,0);
                int e1 = pf.get_edge_or_add(v1,z0,0);
                int e2 = pf.get_edge_or_add(v0,v1,0);

                poly_face f;

                f.addVertex(v0);
                f.addVertex(v1);
                f.addVertex(z0);

                f.addEdge(e0);
                f.addEdge(e1);
                f.addEdge(e2);

                f.row = j;
                f.column = column_no++;

                pf.faces.push_back(f);
                n_f+=1;
                n_t+=1;
            }
        }

       // std::cout << "row " << j << " has " << column_no << " columns\n";
    }

    LineHolder nograph;

    do_primitive_calculations(pf);

    surface_group sg;
    sg.type = SURFACE_GROUP_SPHERE;
    sg.point = vector3df(0, 0, 0);// get_center(pf);
    sg.vec = core::vector3df(0,1,0);
    sg.vec1 = core::vector3df(1,0,0);
    sg.radius = radius;

    pf.surface_groups.push_back(sg);

    for(poly_face& face: pf.faces)
    {
        face.surface_group=0;
    }

    int n_v = pf.vertices.size();
    int n_e = pf.edges.size();

    f32 bands = PI / n_zen_points;
    //std::cout << pf.faces.size() << " / " << n_zen_points << "\n";
    for (poly_face& f : pf.faces)
    {
        f32 theta = atan( f.m_center.Y / sqrt(f.m_center.X * f.m_center.X + f.m_center.Z * f.m_center.Z));
        
        int row_no = floor(theta / bands) + n_zen_points / 2;
        //std::cout << floor(theta / bands) << " ---> " <<row_no << "\n";
        if (row_no >= n_zen_points) std::cout << "ERROR in SPHERE CREATION\n";
        f.row = row_no;
    }

    //v0 = sg->vec;
    //v1 = sg->vec1;

   // sfg = sg;
    //m_height = sg->height;
    //m_radius = sg->radius;

    vector3df iY = sg.vec.crossProduct(sg.vec1);
    iY.normalize();

    for (int i = 0; i < n_zen_points; i++)
    {
        std::vector<poly_face*> face_ptrs;

        for (poly_face& f : pf.faces)
        {
            if (f.row == i && f.vertices.size() > 0)
            {
                face_ptrs.push_back(&f);
            }
        }

        std::sort(face_ptrs.begin(), face_ptrs.end(),
            [&](poly_face* f_a, poly_face* f_b)
            {
                float phi_a = get_angle_phi_for_face(sg.point, sg.vec1, iY, &pf, f_a);
                float phi_b = get_angle_phi_for_face(sg.point, sg.vec1, iY, &pf, f_b);
                return phi_a < phi_b;
            });

        for (int j = 0; j < face_ptrs.size(); j++)
        {
            face_ptrs[j]->column = j;
        }
    }

    return geo;

}

