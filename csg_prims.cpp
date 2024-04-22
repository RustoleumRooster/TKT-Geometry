#include <iostream>
#include "csg_classes.h"

using namespace irr;

core::vector3df get_center(polyfold pf)
{
    core::vector3df ret(0,0,0);
    for(int i=0;i<pf.vertices.size();i++)
        ret+=pf.vertices[i].V;
    ret/=pf.vertices.size();
    return ret;
}

void do_primitive_calculations(polyfold& pf)
{
    for(int f_i=0;f_i<pf.faces.size();f_i++)
    {
        for(int e_i:pf.faces[f_i].edges)
        {
            pf.faces[f_i].addVertex(pf.edges[e_i].v0);
            pf.faces[f_i].addVertex(pf.edges[e_i].v1);
        }
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

    for(int f_i=0;f_i<pf.faces.size();f_i++)
    {
        
        core::vector3df r = pf.faces[f_i].m_center - pf_center;
        f32 dp = pf.faces[f_i].m_normal.dotProduct(r);
        //std::cout << f_i << ": " << dp << "\n";
        if(dp<0)
        {
            pf.faces[f_i].flip_normal();
        }
    }

    pf.recalc_bbox_and_loops();

    pf.topology = TOP_CONVEX;
}

polyfold make_poly_cube(int height,int length,int width)
{
    polyfold pf;

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

    return pf;
}

polyfold make_poly_plane(int length, int width)
{
    polyfold pf;

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

    return pf;
}

polyfold make_cylinder(int height, int radius, int faces)
{
    f32 pi = 3.141592653;
    polyfold pf;

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
    pf.faces.push_back(f);

    f.clear();
    for(int i=0;i<faces-1;i++)
    {
        int a = pf.get_edge_or_add(i*2+1,(i+1)*2+1,0);
        f.addEdge(a);

    }
    aa = pf.get_edge_or_add(1,(faces-1)*2+1,0);
    f.addEdge(aa);
    pf.faces.push_back(f);

    for(int i=0;i<faces-1;i++)
    {
        f.clear();
        int a = pf.get_edge_or_add(i*2,i*2+1,0);
        int b = pf.get_edge_or_add(i*2+1,(i+1)*2+1,0);
        int c = pf.get_edge_or_add((i+1)*2,(i+1)*2+1,0);
        int d = pf.get_edge_or_add(i*2,(i+1)*2,0);
        f.addEdge(a);
        f.addEdge(b);
        f.addEdge(c);
        f.addEdge(d);
        pf.faces.push_back(f);
    }

    f.clear();
    int s = faces-1;
    int a = pf.get_edge_or_add(0*2+1,s*2+1,0);
    int b = pf.get_edge_or_add(0*2,s*2,0);
    int c = pf.get_edge_or_add(0*2,0*2+1,0);
    int d = pf.get_edge_or_add(s*2,s*2+1,0);

    f.addEdge(c);
    f.addEdge(a);
    f.addEdge(d);
    f.addEdge(b);
    pf.faces.push_back(f);

    do_primitive_calculations(pf);

    surface_group sg;

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

    pf.surface_groups.push_back(sg);

    for(poly_face& face: pf.faces)
        face.surface_group=1;

    pf.faces[0].surface_group=0;
    pf.faces[1].surface_group=0;

    return pf;
}

polyfold make_cone(int height, int radius, int faces)
{
    f32 pi = 3.141592653;
    polyfold pf;

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

    return pf;
}

int num_rad_points(int radius, int faces, int zen_faces, int zen_j, bool simplify)
{
    f32 pi = 3.141592653;
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
    f32 pi = 3.141592653;
    int n_zen_points = zen_faces;
    int n_rad_points =  num_rad_points(radius, faces, zen_faces, zen_j, simplify);

    f32 zen_angle = -pi*0.5+(pi/n_zen_points)*zen_j;
    f32 az_angle = (pi*2/n_rad_points)*az_i;

    f32 X = cos(az_angle)*cos(zen_angle)*radius;
    f32 Z = sin(az_angle)*cos(zen_angle)*radius;
    f32 Y = sin(zen_angle)*radius;

    return core::vector3df(X,Y,Z);
}

polyfold make_sphere(int radius, int rad_faces, int zen_faces, bool simplify)
{
    f32 pi = 3.141592653;
    polyfold pf;

    int n_zen_points = zen_faces;
    int n_f=0;
    int n_t=0;

    for(int j=1;j<n_zen_points;j++)
    {
        int n_rad_points = num_rad_points(radius,rad_faces,zen_faces,j,simplify);
        int n_rad_points1 = num_rad_points(radius,rad_faces,zen_faces,j+1,simplify);


        for(int i=0;i<n_rad_points;i++)
        {
            if ( (j>=1 && j< n_zen_points-1) &&
                num_rad_points(radius,rad_faces,zen_faces,j,simplify) == num_rad_points(radius,rad_faces,zen_faces,j+1,simplify))
            {
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

                pf.faces.push_back(f);
                n_f+=1;
                n_t+=1;
            }
            if (j==1)
            {
                int ii = (i+1)%n_rad_points;
                int v0 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j,i,simplify));
                int v1 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,j,ii,simplify));
                int z0 = pf.get_point_or_add(get_sphere_point(radius,rad_faces,zen_faces,0,0,simplify));

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

                pf.faces.push_back(f);
                n_f+=1;
                n_t+=1;
            }
        }
    }

    LineHolder nograph;

    do_primitive_calculations(pf);

    surface_group sg;
    sg.type = SURFACE_GROUP_SPHERE;
    sg.point = get_center(pf);
    sg.vec = core::vector3df(0,1,0);
    sg.vec1 = core::vector3df(0,0,1);
    sg.radius = radius;

    pf.surface_groups.push_back(sg);

    for(poly_face& face: pf.faces)
    {
        face.surface_group=0;
    }

    int n_v = pf.vertices.size();
    int n_e = pf.edges.size();

    return pf;

}

