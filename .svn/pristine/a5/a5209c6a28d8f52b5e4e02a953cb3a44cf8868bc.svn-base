#include "csg_classes.h"

#include <irrlicht.h>
#include <iostream>

/*
bool plane_intersect(plane p0, plane p1, plane& res)
{
        if(abs(abs(p0.norm.dotProduct(p1.norm)) - 1.0) < 0.0001)
        {
            std::cout<<"parallel planes - no intersection\n";
            return false;
        }

        core::vector3df r_dir = p0.norm.crossProduct(p1.norm);
        core::vector3df r = core::vector3df(0,0,0);

        f32 d0,d1;
        d0 = -p0.norm.dotProduct(p0.point);
        d1 = -p1.norm.dotProduct(p1.point);

        if(abs(abs(p1.norm.dotProduct(core::vector3df(0,0,1))) - 1.0) <0.0001 ||
           abs(abs(p1.norm.dotProduct(core::vector3df(0,0,1))) - 1.0) <0.0001)
        {
            if(abs(abs(p1.norm.dotProduct(core::vector3df(0,1,0))) - 1.0) <0.0001 ||
            abs(abs(p1.norm.dotProduct(core::vector3df(0,1,0))) - 1.0) <0.0001)
            {
            //find points along the X=0 plane
                f32 z0,y0,z1,y1;
                z0 = p0.norm.Z;
                y0 = p0.norm.Y;
                z1 = p1.norm.Z;
                y1 = p1.norm.Y;

                r.Y = ((y1*z0)/(y1*z0-z1*y0))*((z1*d0/y1*z0)-(d1/y1));
                r.Z = (-1.0/z0)*(r.Y*y0+d0);
                r.X = 0;

            }
            else
            {
            //find points along the Y=0 plane
                f32 z0,x0,z1,x1;
                z0 = p0.norm.Z;
                x0 = p0.norm.X;
                z1 = p1.norm.Z;
                x1 = p1.norm.X;

                r.Z = ((z1*x0)/(z1*x0-x1*z0))*((x1*d0/z1*x0)-(d1/z1));
                r.X = (-1.0/x0)*(r.Z*z0+d0);
                r.Y = 0;
            }
        }
        else //find points along the Z=0 plane
        {
            f32 x0,y0,x1,y1;
            x0 = p0.norm.X;
            y0 = p0.norm.Y;
            x1 = p1.norm.X;
            y1 = p1.norm.Y;

            r.Y = ((y1*x0)/(y1*x0-x1*y0))*((x1*d0/y1*x0)-(d1/y1));
            r.X = (-1.0/x0)*(r.Y*y0+d0);
            r.Z = 0;
        }

        res.point = r;
        res.norm = r_dir;
        res.norm.normalize();

        return true;
}
*/


/*
bool intersects(const xvertex a0, const xvertex a1, const xvertex b0, const xvertex b1, xvertex& res)
{
    double s1=0;
    double s2=0;
    bool v1=false;
    bool v2=false;

    //std::cout<<"intersect test: ";

    if(a0.x == a1.x)
        v1=true;
    else
    {
        s1 = (a0.y - a1.y) / (a0.x - a1.x);
    }
    if(b0.x == b1.x)
        v2=true;
    else
    {
        s2 = (b0.y - b1.y) / (b0.x - b1.x);
    }

    if((abs(s1-s2) <0.0001 && !v1 && !v2 ) || (v1 && v2))
        {
     //       std::cout<<"parallel - no intersection\n";
            return false;
        }

    if(v1==true)
    {
     //   std::cout<<"line 1 vertical ";
        double yi = s2*a0.x +b0.y-s2*b0.x;

        //if(yi >= std::min(a0.y,a1.y) && yi <= std::max(a0.y,a1.y) && yi >= std::min(b0.y,b1.y) && yi <= std::max(b0.y,b1.y))
        if(yi >= std::min(a0.y,a1.y) && yi <= std::max(a0.y,a1.y) && a0.x >= std::min(b0.x,b1.x) && a0.x <= std::max(b0.x,b1.x))
        {
            res.x = a0.x;
            res.y = yi;
            std::cout<<res.x<<", "<<res.y<<"\n";
            return true;
        }
     //   std::cout<<"no intersection\n";
    }
    else if(v2==true)
    {
     //   std::cout<<"line 2 vertical ";
        double yi = s1*b0.x +a0.y-s1*a0.x;

        //if(yi >= std::min(b0.y,b1.y) && yi <= std::max(b0.y,b1.y) && yi >= std::min(a0.y,a1.y) && yi <= std::max(a0.y,a1.y))
        if(yi >= std::min(b0.y,b1.y) && yi <= std::max(b0.y,b1.y) && b0.x >= std::min(a0.x,a1.x) && b0.x <= std::max(a0.x,a1.x))
        {
            res.x = b0.x;
            res.y = yi;
            std::cout<<res.x<<", "<<res.y<<"\n";
            return true;
        }
    //    std::cout<<"no intersection\n";
    }
    else
    {
        double xi = (s1*a0.x+b0.y-s2*b0.x-a0.y) / (s1 - s2);

        if(xi >= std::min(a0.x,a1.x) && xi <= std::max(a0.x,a1.x) && xi >= std::min(b0.x,b1.x) && xi <= std::max(b0.x,b1.x))
        {
            res.x = xi;
            res.y = s1*xi+a0.y-s1*a0.x;
            std::cout<<res.x<<", "<<res.y<<"\n";
            return true;
        }
     //   std::cout<<"no intersection \n";
    }
}*/


void pfold_intersect_face(polyfold pf1, polyfold pf2, int face_no, polyfold& res_pf)
    {

    core::plane3df iplane(pf2.vertices[pf2.faces[face_no].vertices[0]].V,pf2.faces[face_no].m_normal);

    core::vector3df inter_points[50];
    //core::vector3df ref_edge[50];
    int connections[50];
    int n_connections=0;
     core::line3df aline;

    core::vector3df res;
    int n_inter_points=0;
    bool b= false;

    int added=0;
    for(int f_i =0; f_i < pf1.faces.size(); f_i++)
    {

        added=0;
        for(int e_i =0; e_i < pf1.faces[f_i].edges.size(); e_i++)
        {
        //aline.setLine(pf1.getVertex(f_i,e_i,0).V,pf1.getVertex(f_i,e_i,1).V);
        //graph.lines.push_back(aline);

        b = iplane.getIntersectionWithLimitedLine(pf1.getVertex(f_i,e_i,0).V,
                                                  pf1.getVertex(f_i,e_i,1).V,
                                                  res);
        if(b)
            {
             added++;
             inter_points[n_inter_points]=res;
             //std::cout<<res.X<<","<<res.Y<<","<<res.Z<<"\n";
             n_inter_points++;
            }
        }
        if(added>1)
        {
        connections[n_connections*2]=n_inter_points-2;
        connections[n_connections*2+1]=n_inter_points-1;
        n_connections++;
        //std::cout<<"link\n";
        }
    }
    std::cout<<n_inter_points<<" intersections\n";
    std::cout<<n_connections<<" connections\n";

//    core::line3df aline;

    for(int i=0;i<n_connections;i++)
    {
   //     aline.setLine(inter_points[connections[i*2]],inter_points[connections[i*2+1]]);
   //     std::cout<<aline.start.X<<","<<aline.start.Y<<","<<aline.start.Z<<" --- ";
   //     std::cout<<aline.end.X<<","<<aline.end.Y<<","<<aline.end.Z<<"\n";
   //     graph.lines.push_back(aline);
    }

}

/*
    if(false)
    {
    //core::vector3df v0=my_poly.getVertex(e_i,0).V;
    //core::vector3df v1=my_poly.getVertex(e_i,1).V;
    poly_face face = pf.faces[1];
    std::vector<core::vector3df> verts;
    for(poly_loop loop  : face.loops)
        {
        core::matrix4 R = face.get2Dmat();

        for(int v_i : loop.vertices)
        {
            core::vector3df V = pf.vertices[v_i].V;
            R.rotateVect(V);
            verts.push_back(V);
        }
        verts.push_back(verts[0]);
        for(int i=0;i<verts.size()-1;i++)
            graph.lines.push_back(core::line3df(verts[i],verts[i+1]));

        graph.lines.push_back(core::line3df(core::vector3df(loop.min_x,verts[0].Y,loop.min_z),core::vector3df(loop.min_x,verts[0].Y,loop.max_z)));
        graph.lines.push_back(core::line3df(core::vector3df(loop.max_x,verts[0].Y,loop.min_z),core::vector3df(loop.max_x,verts[0].Y,loop.max_z)));
        graph.lines.push_back(core::line3df(core::vector3df(loop.min_x,verts[0].Y,loop.min_z),core::vector3df(loop.max_x,verts[0].Y,loop.min_z)));
        graph.lines.push_back(core::line3df(core::vector3df(loop.min_x,verts[0].Y,loop.max_z),core::vector3df(loop.max_x,verts[0].Y,loop.max_z)));
        }

    }
*/
/*
void combine_pfold(polyfold pf,polyfold pf2,polyfold& pf3)
{
    pf3.edges = pf2.edges;
    pf3.vertices = pf2.vertices;
    pf3.faces = pf2.faces;

    for(poly_vert vert : pf.vertices)
    {
    pf3.vertices.push_back(poly_vert(vert.V.X,vert.V.Y,vert.V.Z));
    }

    for(poly_edge edge : pf3.edges)
        edge.topo_group=0;

    for(poly_edge edge : pf.edges)
    {
    pf3.edges.push_back(poly_edge(edge.v0+pf2.vertices.size(),edge.v1+pf2.vertices.size(),1));
    }

    poly_face f;
    for(poly_face face : pf.faces)
    {
    f.clear();
    for(int e_i: face.edges)
        {
        f.addEdge(e_i+pf2.edges.size());
        }
    pf3.faces.push_back(f);
    }

    pf3.calc_vertices();

}
*/

/*
int polyfold::left_right_test2(poly_face* face, poly_loop* loop, f32 vx, f32 vz)
{
    std::cout<<"l/r test: "<<loop->vertices.size()<<"   "<<vx<<","<<vz<<"\n";
    core::vector3df point;
    point.X = vx;
    point.Z = vz;

    f32 closest_x=99000;
    f32 closest_z=99000;
    int status=-1; //-1 = undefined, 0 = left, 1 = right
    std::vector<core::vector3df> verts;
    for(int v_i : loop->vertices)
    {
        verts.push_back(this->vertices[v_i].V);
    }
    verts.push_back(verts[0]);

    core::matrix4 R = face->get2Dmat();
    for(core::vector3df &v : verts)
    {
        R.rotateVect(v);
    }
    for(int i = 0 ; i<verts.size()-1; i++)
    {
        core::vector3df leftv,rightv;
        if(verts[i].X < verts[i+1].X)
        {
            leftv = verts[i];
            rightv = verts[i+1];
        }
        else
        {
            leftv = verts[i+1];
            rightv = verts[i];
        }

        if(point.X > leftv.X &&
           point.X < rightv.X)
        {
            f32 m = (rightv.Z - leftv.Z)/ (rightv.X - leftv.X);
            f32 z_intersect = m*(point.X-leftv.X)+leftv.Z;

            if(verts[i].X < verts[i+1].X
               && point.Z > z_intersect
               && abs(point.Z - z_intersect)<closest_z)
            {
                status = 0; //Left
                closest_z = abs(point.Z - z_intersect);
            }
            else if(verts[i].X < verts[i+1].X
               && point.Z < z_intersect
               && abs(point.Z - z_intersect)<closest_z)
            {
                status = 1; //Right
                closest_z = abs(point.Z - z_intersect);
            }
            else if(verts[i].X > verts[i+1].X
               && point.Z > z_intersect
               && abs(point.Z - z_intersect)<closest_z)
            {
                status = 1; //Right
                closest_z = abs(point.Z - z_intersect);
            }
            else if(verts[i].X > verts[i+1].X
               && point.Z < z_intersect
               && abs(point.Z - z_intersect)<closest_z)
            {
                status = 0; //Left
                closest_z = abs(point.Z - z_intersect);
            }
        }
        if (point.Z > std::min(verts[i].Z,verts[i+1].Z) &&
           point.Z < std::max(verts[i].Z,verts[i+1].Z))
        {
            f32 m = (rightv.Z - leftv.Z)/ (rightv.X - leftv.X);
            f32 x_intersect = ((point.Z-leftv.Z))/m+leftv.X;
            if(verts[i].Z < verts[i+1].Z
               && point.X < x_intersect
               && abs(point.X - x_intersect)<closest_x)
            {
                status = 0; //Left
                closest_x = abs(point.X - x_intersect);
            }
            else if(verts[i].Z < verts[i+1].Z
               && point.X > x_intersect
               && abs(point.X - x_intersect)<closest_x)
            {
                status = 1; //Right
                closest_x = abs(point.X - x_intersect);
            }
            else if(verts[i].Z > verts[i+1].Z
               && point.X < x_intersect
               && abs(point.X - x_intersect)<closest_x)
            {
                status = 1; //Right
                closest_x = abs(point.X - x_intersect);
            }
            else if(verts[i].Z > verts[i+1].Z
               && point.X > x_intersect
               && abs(point.X - x_intersect)<closest_x)
            {
                status = 0; //Left
                closest_x = abs(point.X - x_intersect);
            }
        }
    }
    if(status == -1)
    {
        std::cout<<"warning - undefined left right test\n";
        for(core::vector3df v : verts)
        {
            std::cout<<v.X<<","<<v.Z<<" ";
        }
        std::cout<<"("<<vx<<","<<vz<<")";
        std::cout<<"\n";
    }
   return status;
}
*/

/*
    polyfold sort;
    sort.vertices.push_back(poly_vert(36,0,0));
    sort.vertices.push_back(poly_vert(23,0,0));
    sort.vertices.push_back(poly_vert(4,0,0));
    sort.vertices.push_back(poly_vert(60,0,0));
    sort.vertices.push_back(poly_vert(-15,0,0));
    sort.vertices.push_back(poly_vert(100,0,0));

    sort_inline_vertices(sort);
    for(poly_vert : sort.vertices)
    {
        std::cout<<poly_vert.V.X<<" ";
    }
    std::cout<<"\n";
*/

/*

    if(false)
    {
    //core::vector3df v0=my_poly.getVertex(e_i,0).V;
    //core::vector3df v1=my_poly.getVertex(e_i,1).V;
    poly_face face = pf3.faces[0];
    core::matrix4 R = face.get2Dmat();
    std::vector<core::vector3df> verts;
    core::vector3df v0 = core::vector3df(0,0,70.76);
    core::vector3df v1 = core::vector3df(300,0,70.76);


    std::cout<<v0.X<<","<<v0.Y<<","<<v0.Z<<".\n";
    std::cout<<v1.X<<","<<v1.Y<<","<<v1.Z<<".\n";
    //std::cout<<loop.min_x<<","<<loop.max_x<<"  "<<loop.min_z<<","<<loop.max_z<<"\n";
    R.rotateVect(v0);
    R.rotateVect(v1);
    for(poly_loop loop  : face.loops)
        {

        //0,0,70.76
        //300 0 70.76

        for(int v_i : loop.vertices)
        {
            core::vector3df V = pf3.vertices[v_i].V;
            R.rotateVect(V);
            verts.push_back(V);
        }
        verts.push_back(verts[0]);
        for(int i=0;i<verts.size()-1;i++)
            graph.lines.push_back(core::line3df(verts[i],verts[i+1]));

       // if(pf3.left_right_test2(&face,&loop,v0.X,v0.Z)) std::cout<<"yes\n";
       // if(pf3.is_point_in_loop(&face,&loop,v1)) std::cout<<"yes\n";

        graph.lines.push_back(core::line3df(core::vector3df(loop.min_x,verts[0].Y,loop.min_z),core::vector3df(loop.min_x,verts[0].Y,loop.max_z)));
        graph.lines.push_back(core::line3df(core::vector3df(loop.max_x,verts[0].Y,loop.min_z),core::vector3df(loop.max_x,verts[0].Y,loop.max_z)));
        graph.lines.push_back(core::line3df(core::vector3df(loop.min_x,verts[0].Y,loop.min_z),core::vector3df(loop.max_x,verts[0].Y,loop.min_z)));
        graph.lines.push_back(core::line3df(core::vector3df(loop.min_x,verts[0].Y,loop.max_z),core::vector3df(loop.max_x,verts[0].Y,loop.max_z)));
        std::cout<<"bounds: "<<loop.min_x<<","<<loop.max_x<<"  "<<loop.min_z<<","<<loop.max_z<<"\n";
        }

   // graph.points.push_back(v0);
    graph.points.push_back(v1);
    }
*/
/*
for(int face_i=0;face_i<pf.faces.size();face_i++)
        for(int face_j=0;face_j<pf2.faces.size();face_j++)
    {
        poly_face f=pf.faces[face_i];
        poly_face f2=pf2.faces[face_j];

        plane res;

        core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
        core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);

        if(f_plane.getIntersectionWithPlane(f2_plane,res.point,res.norm))
        {
            core::vector3df ipoint;
            //----
            //graph.lines.push_back(core::line3df(res.point-res.norm*1000,res.point+res.norm*1000));

            for(int e_i : pf4.faces[face_i].edges)
            {
                core::vector3df v0=pf4.getVertex(e_i,0).V;
                core::vector3df v1=pf4.getVertex(e_i,1).V;
                if(f2_plane.getIntersectionWithLimitedLine(v0,v1,ipoint))
                {
                    //graph.points.push_back(ipoint);
                    if(pf2.is_point_on_face(&f2,ipoint))
                       {
                           int new_v = pf4.get_point_or_add(ipoint);
                          // graph.points.push_back(ipoint);
                           num++;
                           // std::cout<<"ipoint: "<<ipoint.X<<","<<ipoint.Y<<","<<ipoint.Z<<"\n";
                           // graph.lines.push_back(core::line3df(pf4.getVertex(e_i,1).V,pf4.getVertex(e_i,0).V));
                           if(f2_plane.classifyPointRelation(v0)==core::ISREL3D_FRONT)
                           {
                              pf4.bisect_edge(e_i,new_v,0,1);
                           }
                           else
                           {
                             pf4.bisect_edge(e_i,new_v,1,0);
                           }
                       }
                }
            }

             for(int e_i : pf5.faces[face_j].edges)
            {
                core::vector3df v0=pf5.getVertex(e_i,0).V;
                core::vector3df v1=pf5.getVertex(e_i,1).V;

               // if(e_i<2)
               // graph.lines.push_back(core::line3df(pf5.getVertex(e_i,1).V,pf5.getVertex(e_i,0).V));

                if(f_plane.getIntersectionWithLimitedLine(v0,v1,ipoint))
                {
                    if(pf.is_point_on_face(&f,ipoint))
                       {
                           num++;
                          // graph.points.push_back(ipoint);
                          // std::cout<<"ipoint2: "<<ipoint.X<<","<<ipoint.Y<<","<<ipoint.Z<<"\n";
                           int new_v = pf5.get_point_or_add(ipoint);

                           //v0=pf5.getVertex(e_i,0).V

                           if(f_plane.classifyPointRelation(v0)==core::ISREL3D_FRONT)
                           {
                              pf5.bisect_edge(e_i,new_v,0,1);
                           }
                           else
                           {
                             pf5.bisect_edge(e_i,new_v,1,0);
                           }
                       }
                }
            }
        }
    }
*/

/*
    for(int face_i=0;face_i<pf.faces.size();face_i++)
        for(int face_j=0;face_j<pf2.faces.size();face_j++)
    {
        poly_face f=pf.faces[face_i];
        poly_face f2=pf2.faces[face_j];

        plane res;

        core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
        core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);

        //if(!f_plane.getIntersectionWithPlane(f2_plane,res.point,res.norm))
        //    std::cout<<"nope\n";

        if(f_plane.getIntersectionWithPlane(f2_plane,res.point,res.norm))
        {
            core::matrix4 R= f.get2Dmat();

            core::matrix4 R_inv;
            R.getInverse(R_inv);

            f32 Y_Value;
            core::vector3df avec = pf.getVertex(f.edges[0],0).V;
            R.rotateVect(avec);
            Y_Value = avec.Y;

            std::vector<core::line2df> lines;

            core::vector3df v0,v1;
            for(int e_i : f.edges)
            {
                v0=pf.getVertex(e_i,0).V;
                v1=pf.getVertex(e_i,1).V;
                R.rotateVect(v0);
                R.rotateVect(v1);
                lines.push_back(core::line2df(v0.X,v0.Z,v1.X,v1.Z));
            }

            //v0=res.point-res.norm*9000;
            //v1=res.point+res.norm*9000;
           // graph.lines.push_back(core::line3df(v0,v1));

            R.rotateVect(res.point);
            R.rotateVect(res.norm);
            v0=res.point-res.norm*99000;
            v1=res.point+res.norm*99000;
            core::line2df intersection_line(v0.X,v0.Z,v1.X,v1.Z);

            core::vector2df intersection_result;
            std::vector<core::vector3df> intersection_points;

            for(int e_i = 0; e_i<lines.size(); e_i++)
            {
            core::line2df e_line = lines[e_i];
           // if(intersection_line.intersectWith(e_line,intersection_result))
           if(do_lines_intersect(intersection_line,e_line,intersection_result))
               {
                core::vector3df v = core::vector3df(intersection_result.X,Y_Value,intersection_result.Y);
                intersection_points.push_back(v);
                //if(face_i==1)
                   // std::cout<<"v1: "<<v.X<<","<<v.Y<<","<<v.Z<<"\n";
               }
            }


            order_line_intersections(v0,intersection_points);
            for(core::vector3df &v : intersection_points)
            {
                R_inv.rotateVect(v);
               // if(face_i==1)
               //     graph.points.push_back(v);
            }
           // std::cout<<"...\n";
            //=====================

            R = f2.get2Dmat();
            R.getInverse(R_inv);

            avec = pf2.getVertex(f2.edges[0],0).V;
            R.rotateVect(avec);
            Y_Value = avec.Y;
            std::vector<core::vector3df> line_segments = intersection_points; //pf
            std::vector<core::line2df> lines2;

            lines.clear();
            for(int e_i : f2.edges)
            {
                v0=pf2.getVertex(e_i,0).V;
                v1=pf2.getVertex(e_i,1).V;
                R.rotateVect(v0);
                R.rotateVect(v1);
                lines.push_back(core::line2df(v0.X,v0.Z,v1.X,v1.Z));
             //   std::cout<<"v2: "<<v0.X<<","<<v0.Z<<"..."<<v1.X<<","<<v1.Z<<"\n";
            }

            for(core::vector3df &v : line_segments)
            {
                R.rotateVect(v);
            }

            for(int i=0; i<line_segments.size()/2; i++)
            {
                lines2.push_back(core::line2df(line_segments[i*2].X,
                                               line_segments[i*2].Z,
                                               line_segments[i*2+1].X,
                                               line_segments[i*2+1].Z));
            }

            for(core::line2df line_a : lines2) //pf
            {
                intersection_points.clear();


               // std::cout<<".v0: "<<line_a.start.X<<","<<line_a.start.Y<<"\n";
               // std::cout<<".v1: "<<line_a.end.X<<","<<line_a.end.Y<<"\n";
               // std::cout<<"len "<<line_a.getLength()<<"\n";

                if(line_a.getLength() > 0.1)
                {
                    intersection_points.push_back(core::vector3df(line_a.start.X,Y_Value,line_a.start.Y));
                    intersection_points.push_back(core::vector3df(line_a.end.X,Y_Value,line_a.end.Y));

                    for(core::line2df line_b : lines) //pf2
                    {

                        if(line_a.intersectWith(line_b,intersection_result))
                        {
                        if( (is_same_point(intersection_result.X,line_a.start.X) && is_same_point(intersection_result.Y, line_a.start.Y)) ||
                            (is_same_point(intersection_result.X,line_a.end.X) && is_same_point(intersection_result.Y,line_a.end.Y))  )
                           {
                         //   std::cout<<"redundant intersection\n";
                           }
                        else
                         intersection_points.push_back(core::vector3df(intersection_result.X,Y_Value,intersection_result.Y));
                        // std::cout<<".v2: "<<intersection_result.X<<","<<intersection_result.Y<<"\n";
                        }
                    }


                order_line_intersections(intersection_points[0],intersection_points);

                std::vector<int> edge_points1;
                std::vector<int> edge_points2;

                 for(int i =0; i<intersection_points.size()-1; i++)
                {
                    v0= intersection_points[i];
                    v1= intersection_points[i+1];
                    R_inv.rotateVect(v0);
                    R_inv.rotateVect(v1);
                   // std::cout<<"v0: "<<v0.X<<","<<v0.Y<<","<<v0.Z<<"\n";
                   // std::cout<<"v1: "<<v1.X<<","<<v1.Y<<","<<v1.Z<<"\n";
                    //graph.points.push_back(core::line3df(v0,v1));
                   // graph.points.push_back(v0);
                   // graph.points.push_back(v1);
                }

                for(int i =0; i<intersection_points.size(); i++)
                {
                    v0= intersection_points[i];
                    R_inv.rotateVect(v0);

                    int v_i1 = pf4.find_point(v0);
                    int v_i2 = pf5.find_point(v0);
                   // int v_i1 = pf4.get_point_or_add(v0);
                   // int v_i2 = pf5.get_point_or_add(v0);

                    if(v_i1 != -1 )
                    {
                        //graph.points.push_back(v0);
                            edge_points1.push_back(pf4.get_point_or_add(pf4.vertices[v_i1].V));
                            edge_points2.push_back(pf5.get_point_or_add(pf4.vertices[v_i1].V));
                    }
                    else if(v_i2 != -1 )
                    {
                        //graph.points.push_back(v0);
                            edge_points1.push_back(pf4.get_point_or_add(pf5.vertices[v_i2].V));
                            edge_points2.push_back(pf5.get_point_or_add(pf5.vertices[v_i2].V));
                    }
                }
                //  poly_face f=pf.faces[face_i];
                //  poly_face f2=pf2.faces[face_j];
              //  std::cout<<"size is "<<edge_points1.size()<<"\n";
                if(edge_points1.size()>0)
                    for(int i=0;i<edge_points1.size()-1;i++)
                    {
                           // f.addEdge()
                         //  ret.edges.push_back( poly_edge(edge_points1[i],edge_points1[i+1],3));
                        if(!pf4.edge_exists(edge_points1[i],edge_points1[i+1]))
                        {

                           // std::cout<<face_i<<","<<face_j<<" add edge "<<edge_points2[i]<<" to "<<edge_points2[i+1]<<"\n";
                          //  pf4.edges.push_back(poly_edge(edge_points1[i],edge_points1[i+1],3));
                          //  pf4.faces[face_i].edges.push_back(pf4.edges.size()-1);
                        }

                        if(!pf5.edge_exists(edge_points2[i],edge_points2[i+1]))
                        {
                           // std::cout<<"b";
                          //  pf5.edges.push_back(poly_edge(edge_points2[i],edge_points2[i+1],3));
                          //  pf5.faces[face_j].edges.push_back(pf5.edges.size()-1);
                        }
                        num++;


                      //  std::cout<<face_i<<","<<face_j<<" add edge "<<edge_points2[i]<<" to "<<edge_points2[i+1]<<"\n";
                       i++;
                    }
                }
            }
        }
    }*/

    /*
            while(still_has_edges)
            {
                temp_vec.clear();

                first_v = this->edges[cur_edge].v0;
                cur_v = this->edges[cur_edge].v1;

                temp_vec.push_back(first_v);
                temp_vec.push_back(cur_v);

                //edge_loop_no[cur_edge]=0;

                while(cur_v != first_v)
                {
                    for(int i=0; i< face.edges.size(); i++)
                    {
                    int e_i = face.edges[i];
                    if( e_i != cur_edge &&this->edges[e_i].v0 == cur_v && edge_loop_no[i] != 0)
                        {
                        cur_edge = e_i;
                        edge_loop_no[i]=0;
                        cur_v = this->edges[e_i].v1;
                        if(cur_v != first_v)
                            temp_vec.push_back(cur_v);
                        }
                    else if( e_i != cur_edge && this->edges[e_i].v1 == cur_v && edge_loop_no[i] != 0)
                        {
                        cur_edge = e_i;
                        edge_loop_no[i]=0;
                        cur_v = this->edges[e_i].v0;
                        if(cur_v != first_v)
                            temp_vec.push_back(cur_v);
                        }
                    }
                }

                poly_loop aloop;
                aloop.vertices=temp_vec;
                face.loops.push_back(aloop);
                //std::cout<<"Added loop with "<<temp_vec.size()<<" vertices\n";

                still_has_edges=false;
                int next_e;
                for(int i = 0; i< edge_loop_no.size(); i++)
                {
                     if(edge_loop_no[i]==-1)
                     {
                         still_has_edges = true;
                         next_e = i;
                     }
                }
                if(still_has_edges)
                {
                   edge_loop_no[next_e]=0;
                   cur_edge = face.edges[next_e];
                }
            }
        }*/

/*

bool do_lines_intersect(core::line2df line0, core::line2df line1, core::vector2df& out)
{
    core::vector2df v;
    if(line0.intersectWith(line1,out))
       return true;
    else if ((v = line0.getClosestPoint(line1.start)-line1.start).getLength()<0.5)
    {
        out = v;
        return true;
    }
    else if ((v = line0.getClosestPoint(line1.end)-line1.end).getLength()<0.5)
    {
        out = v;
        return true;
    }
    else if ((v = line1.getClosestPoint(line0.start)-line0.start).getLength()<0.5)
    {
        out = v;
        return true;
    }
    else if ((v = line1.getClosestPoint(line0.end)-line0.end).getLength()<0.5)
    {
        out = v;
        return true;
    }
    return false;
}
*/

/*

void order_line_intersections(core::vector3df origin, std::vector<core::vector3df>& intersection_points)
{
    std::vector<core::vector3df> temp_vec0;
    std::vector<f32> temp_vec1;

    if(intersection_points.size() == 0) return;

    temp_vec0=intersection_points;
    temp_vec1.resize(intersection_points.size());

    for(int i=0;i<intersection_points.size();i++)
    {
     temp_vec1[i]=origin.getDistanceFrom(intersection_points[i]);
    }
    int best=0;
    f32 best_dist=temp_vec1[0];
    f32 so_far;
    core::vector3df tv;
    f32 tf;

    for(int i=0;i<intersection_points.size()-1;i++)
    {
        for(int j=i+1;j<intersection_points.size();j++)
        {
        if( temp_vec1[j]<temp_vec1[i])
            {
            tf=temp_vec1[i];
            tv=temp_vec0[i];
            temp_vec1[i]=temp_vec1[j];
            temp_vec0[i]=temp_vec0[j];
            temp_vec1[j]=tf;
            temp_vec0[j]=tv;
            }
        }
    }

    intersection_points=temp_vec0;
}
*/

/*
 if(f.edges.size()>2)
                    if(f.edges.size()>2)
            {
                bool doDelete = false;

                doDelete=true;
                for(int e_i : f.edges)
                    {
                    int e_j = pf5.find_edge(ret.getVertex(e_i,0).V,ret.getVertex(e_i,0).V);
                    if(e_j == -1 || pf5.edges[e_j].topo_group == 0)
                        {
                        doDelete=false;
                        }
                    }

                for(int e_i : f.edges)
                    {
                    int e_j = pf5.find_edge(ret.getVertex(e_i,1).V,ret.getVertex(e_i,0).V);
                    if(e_j != -1 && pf5.edges[e_j].topo_group ==0)
                        {
                        doDelete=true;
                        }
                    }

               // if(!doDelete)
                {
                 ret.faces.push_back(f);
                }
            }
*/
/*

        for(int f_i=0; f_i < pf5.faces.size(); f_i++)
        {
            f.clear();
            num=0;

            pf5.faces[f_i].loops.clear();
            //pf5.calc_loops(pf5.faces[f_i],3);
            //pf5.calc_loops(pf5.faces[f_i],rule);
            //pf5.calc_loops(pf5.faces[f_i],rule+4);

            std::cout<<"face "<<f_i<<": "<<pf5.faces[f_i].loops.size()<<" loops: ";
            for(poly_loop loop: pf5.faces[f_i].loops)
                std::cout<<loop.vertices.size()<<" ";
            std::cout<<"\n";

            pf5.sort_loops(pf5.faces[f_i]);

            f.m_normal = pf5.faces[f_i].m_normal;
            f.bFlippedNormal = pf5.faces[f_i].bFlippedNormal;

            for(int e_i : pf5.faces[f_i].edges)
            {
                if(pf5.edges[e_i].topo_group==rule || pf5.edges[e_i].topo_group==3  || pf5.edges[e_i].topo_group==5)
                {
                   // std::cout<<pf5.getVertex(e_i,0).V.X<<","<<pf5.getVertex(e_i,0).V.Y<<","<<pf5.getVertex(e_i,0).V.Z<<" to "<<pf5.getVertex(e_i,1).V.X<<","<<pf5.getVertex(e_i,1).V.Y<<","<<pf5.getVertex(e_i,1).V.Z<<"\n";

                    int v_i0 = ret.get_point_or_add(pf5.getVertex(e_i,0).V);
                    int v_i1 = ret.get_point_or_add(pf5.getVertex(e_i,1).V);
                    int new_e = ret.get_edge_or_add(v_i0,v_i1,pf5.edges[e_i].topo_group);
                    f.addEdge(new_e);
                }
            }

            for(poly_loop loop : pf5.faces[f_i].loops)
            {
                poly_loop new_loop;
                for(int v_i : loop.vertices)
                    new_loop.vertices.push_back(ret.get_point_or_add(pf5.vertices[v_i].V));
                new_loop.type = loop.type;
                ret.calc_loop_bbox(f,new_loop);
                f.loops.push_back(new_loop);
            }


            //std::cout<<"\n";

            if(f.edges.size()>2 && f.loops.size() > 0)
                ret.faces.push_back(f);
        }
        */


/*
            for(int f_j=0; f_j < pf5.faces.size(); f_j++)
            {
             if(pf4.point_is_coplanar(f_i,pf5.faces[f_j].m_center) &&
                pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal) > 0.99)
                {
                    std::cout<<"coplanar\n";
                    for(int e_i : pf4.faces[f_i].edges)
                    {
                        if(pf4.edges[e_i].topo_group==5)
                        {
                        core::vector3df v0 = pf4.getVertex(e_i,0).V;
                        core::vector3df v1 = pf4.getVertex(e_i,1).V;

                            if(pf5.is_point_on_face(&pf5.faces[f_j],v0) && pf5.is_point_on_face(&pf5.faces[f_j],v1))
                                pf4.edges[e_i].topo_group=!rule;
                        }
                    }
                }
            }
*/

/*

            std::cout<<pf4.faces[f_i].edges.size()<<"\n";
            std::vector<int> tvec;
            for(int e_i : pf4.faces[f_i].edges)
            {
                bool b0=false;
                bool b1=false;
                if(pf4.edges[e_i].topo_group==3)
                {
                    for(int e_j : pf4.faces[f_i].edges)
                    {
                    if(e_i != e_j && (pf4.edges[e_j].topo_group == rule || pf4.edges[e_j].topo_group==3))
                        {
                            if(pf4.edges[e_i].v0 == pf4.edges[e_j].v0 || pf4.edges[e_i].v0 == pf4.edges[e_j].v1)
                               b0= true;
                            else if(pf4.edges[e_i].v1 == pf4.edges[e_j].v0 || pf4.edges[e_i].v1 == pf4.edges[e_j].v1)
                               b1= true;
                        }
                    }
                if(b0 && b1)
                    tvec.push_back(e_i);
                }
                else
                    tvec.push_back(e_i);
            }
            pf4.faces[f_i].edges=tvec;
            std::cout<<pf4.faces[f_i].edges.size()<<"\n";
*/

/*

void combine_pfold(polyfold pf,polyfold pf2,polyfold& pf3)
{
    pf3.edges = pf2.edges;
    pf3.vertices = pf2.vertices;
    pf3.faces = pf2.faces;

    for(poly_vert vert : pf.vertices)
    {
    pf3.vertices.push_back(poly_vert(vert.V.X,vert.V.Y,vert.V.Z));
    }

    for(poly_edge edge : pf3.edges)
        edge.topo_group=0;

    for(poly_edge edge : pf.edges)
    {
    pf3.edges.push_back(poly_edge(edge.v0+pf2.vertices.size(),edge.v1+pf2.vertices.size(),1));
    }

    poly_face f;
    for(poly_face face : pf.faces)
    {
    f.clear();
    f.m_normal= face.m_normal;
    f.m_center= face.m_center;

    for(int e_i: face.edges)
        {
        f.addEdge(e_i+pf2.edges.size());
        }
    pf3.faces.push_back(f);
    }
    pf3.calc_vertices();

}*/

/*
bool polyfold::point_is_in_front_of_face(int face, core::vector3df v )
{
    core::plane3df plane = core::plane3df(this->faces[face].m_center,this->faces[face].m_normal);
    if(this->topology==TOP_CONVEX)
    {
        if(plane.getDistanceTo(v)<0.2)
            return false;
        else if(plane.classifyPointRelation(v)==core::ISREL3D_FRONT)
            return true;
        else return false;
    }
   else
   {
       if(abs(plane.getDistanceTo(v))<0.2)
            return false;
        else if(plane.classifyPointRelation(v)==core::ISREL3D_BACK)
            return true;
        else return false;
   }

}

bool polyfold::point_is_behind_edge(int edge, core::vector3df v )
{
   // std::cout<<"edge: "<<edge<<"\n";
   //if(this->topology==TOP_CONVEX)
   if(true)
   {
        for(int f_i=0; f_i<this->faces.size(); f_i++)
        {
            for(int e_i : this->faces[f_i].edges)
            {
                if(e_i == edge && this->point_is_in_front_of_face(f_i,v)==true)
                    return false;
            }
        }
        return true;
   }
   else
   {
       for(int f_i=0; f_i<this->faces.size(); f_i++)
        {
            for(int e_i : this->faces[f_i].edges)
            {
                if(e_i == edge && this->point_is_in_front_of_face(f_i,v)==false)
                    return true;
            }
        }
        return false;
   }
}
*/
/*
bool polyfold::point_is_behind_point(int point, core::vector3df v )
{
    //if(this->topology==TOP_CONVEX)
    if(true)
    {
        for(int f_i=0; f_i<this->faces.size(); f_i++)
        {
            for(int v_i : this->faces[f_i].vertices)
            {
                if(v_i == point && this->point_is_in_front_of_face(f_i,v)==true)
                    return false;
            }
        }
        return true;
    }
    else
    {
        //std::cout<<"start: "<<v.X<<","<<"v.Y"<<","<<v.Z<<"\n";
        for(int f_i=0; f_i<this->faces.size(); f_i++)
        {
            for(int v_i : this->faces[f_i].vertices)
            {
                if(v_i == point)
                {
                //std::cout<<f_i<<" ";
                //if(v_i == point && this->point_is_in_front_of_face(f_i,v)==true)
                //    std::cout<<"1\n";
                //else
                //    std::cout<<"0\n";
                if(v_i == point && this->point_is_in_front_of_face(f_i,v)==false)
                    return true;
                }
            }
        }
        return false;
    }
}
*/

/*


    if(false)
    for(int face_j=0;face_j<pf2.faces.size();face_j++)
    {
        poly_face f2=pf2.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);
        core::vector3df ipoint;

        for(int e_i=0; e_i<pf4.edges.size(); e_i++)
        {
            core::vector3df v0=pf4.getVertex(e_i,0).V;
            core::vector3df v1=pf4.getVertex(e_i,1).V;
            if(line_intersects_plane(f2_plane,v0,v1,ipoint) && pf4.edges[e_i].topo_group != -1)
            {
                //if(face_j == 4)
                //    graph.points.push_back(ipoint);
                if(pf2.is_point_on_face(&f2,ipoint) )
                {
                    //graph.points.push_back(ipoint);
                   // if(face_j == 3)
                   // {
                   //  graph.points.push_back(ipoint);
                   //  graph.lines.push_back(core::line3df(pf4.getVertex(e_i,1).V,pf4.getVertex(e_i,0).V));
                   // }
                    num++;
                    // std::cout<<"ipoint: "<<ipoint.X<<","<<ipoint.Y<<","<<ipoint.Z<<"\n";
                    // graph.lines.push_back(core::line3df(pf4.getVertex(e_i,1).V,pf4.getVertex(e_i,0).V));
                    if(pf.find_point(ipoint) != -1)
                    {

                        int new_v = pf4.get_point_or_add(ipoint);
                        core::vector3df other_end;
                        if(new_v == pf4.edges[e_i].v0)
                        {
                            core::vector3df r = v1 - v0;
                            r.normalize();
                            r*=100;
                            other_end = v0+r;
                        }
                        else
                        {
                            core::vector3df r = v0 - v1;
                            r.normalize();
                            r*=100;
                            other_end = v1+r;
                        }
                        //point on point
                        int double_point=pf2.find_point(ipoint);
                        if(double_point != -1)
                        {

                            //graph.points.push_back(ipoint);
                            //std::cout<<double_point<<" d \n";
                            //std::cout<<"ipoint: "<<ipoint.X<<","<<ipoint.Y<<","<<ipoint.Z<<"\n";
                            //std::cout<<"to: "<<other_end.X<<","<<other_end.Y<<","<<other_end.Z<<"\n";
                            if(pf2.point_is_behind_point2(double_point,other_end) == TOP_BEHIND)
                                pf4.edges[e_i].topo_group=1;
                            else if(pf2.point_is_behind_point2(double_point,other_end) == TOP_FRONT)
                                pf4.edges[e_i].topo_group=0;
                        }
                        else
                        {

                            int myedge=-1;
                            for(int e_j=0; e_j<pf2.edges.size(); e_j++)
                            {
                                if(pf2.point_is_on_edge(e_j,ipoint))
                                    myedge=e_j;
                            }
                            //point on edge
                            if(myedge != -1)
                            {
                                //graph.points.push_back(ipoint);
                                //graph.points.push_back(ipoint);
                                //std::cout<<"ipoint: "<<ipoint.X<<","<<ipoint.Y<<","<<ipoint.Z<<"\n";
                                //std::cout<<"to: "<<other_end.X<<","<<other_end.Y<<","<<other_end.Z<<"\n";
                                if(pf2.point_is_behind_edge(myedge,other_end))
                                {
                                    pf4.edges[e_i].topo_group=1;
                                }
                                else
                                    pf4.edges[e_i].topo_group=0;

                                new_v = pf5.get_point_or_add(ipoint);
                                pf5.faces[face_j].addVertex(new_v);

                            }
                            else    //point on face
                            {
                               // graph.points.push_back(ipoint);
                                //std::cout<<e_i<<" "<<face_j<<"\n";
                                //std::cout<<"ipoint: "<<ipoint.X<<","<<ipoint.Y<<","<<ipoint.Z<<"\n";
                                //std::cout<<"to: "<<other_end.X<<","<<other_end.Y<<","<<other_end.Z<<"\n";
                                if(pf2.point_is_in_front_of_face(face_j,other_end))
                                    pf4.edges[e_i].topo_group=0;
                                else
                                    pf4.edges[e_i].topo_group=1;

                                new_v = pf5.get_point_or_add(ipoint);
                                pf5.faces[face_j].addVertex(new_v);
                            }
                        }
                    }
                    else
                    {

                        int double_point=pf2.find_point(ipoint);

                        //edge through point
                        if(double_point != -1)
                        {
                            //graph.points.push_back(ipoint);
                            //std::cout<<e_i<<"...\n";
                            //std::cout<<pf2.point_is_behind_point(double_point,v0)<<"\n";
                            //std::cout<<pf2.point_is_behind_point(double_point,v1)<<"\n";

                            //int new_v = pf4.get_point_or_add(ipoint);
                            //pf4.faces[face_j].addVertex(new_v);

                           // graph.points.push_back(ipoint);
                           // std::cout<<double_point<<"!\n";

                           //std::cout<<"v0: "<<v0.X<<","<<v0.Y<<","<<v0.Z<<"\n";
                           // std::cout<<"v1: "<<v1.X<<","<<v1.Y<<","<<v1.Z<<"\n";

                            if(pf2.point_is_behind_point2(double_point,v0) == TOP_BEHIND &&
                               pf2.point_is_behind_point2(double_point,v1) == TOP_BEHIND)
                                {
                                   // std::cout<<"ipoint: "<<ipoint.X<<","<<ipoint.Y<<","<<ipoint.Z<<"\n";


                                    //pf4.edges[e_i].topo_group=1;

                                    int new_v = pf4.get_point_or_add(ipoint);
                                    pf4.bisect_edge(e_i,new_v,1,1);

                                    new_v = pf5.get_point_or_add(ipoint);
                                    pf5.faces[face_j].addVertex(new_v);
                                }
                            else if(pf2.point_is_behind_point2(double_point,v0) == TOP_FRONT&&
                                    pf2.point_is_behind_point2(double_point,v1) == TOP_FRONT)
                                {
                                    //pf4.edges[e_i].topo_group=0;

                                    int new_v = pf4.get_point_or_add(ipoint);
                                    pf4.bisect_edge(e_i,new_v,0,0);

                                    new_v = pf5.get_point_or_add(ipoint);
                                    pf5.faces[face_j].addVertex(new_v);
                                }
                            else if(pf2.point_is_behind_point2(double_point,v0) == TOP_BEHIND)
                                {
                                    //graph.points.push_back(ipoint);
                                    int new_v = pf4.get_point_or_add(ipoint);
                                    pf4.bisect_edge(e_i,new_v,1,0);
                                   // graph.points.push_back(ipoint);
                                }
                            else if(pf2.point_is_behind_point2(double_point,v0) == TOP_FRONT)
                                {
                                    int new_v = pf4.get_point_or_add(ipoint);
                                    pf4.bisect_edge(e_i,new_v,0,1);
                                    //graph.points.push_back(ipoint);
                                }
                            // TESTING
                            else if(pf2.point_is_in_front_of_face(face_j,v0))
                            {
                                //graph.points.push_back(ipoint);
                                int new_v = pf4.get_point_or_add(ipoint);
                                pf4.bisect_edge(e_i,new_v,0,1);

                                new_v = pf5.get_point_or_add(ipoint);
                                pf5.faces[face_j].addVertex(new_v);
                            }
                            else
                            {
                                //graph.points.push_back(ipoint);
                                int new_v = pf4.get_point_or_add(ipoint);
                                pf4.bisect_edge(e_i,new_v,1,0);

                                new_v = pf5.get_point_or_add(ipoint);
                                pf5.faces[face_j].addVertex(new_v);
                            }
                        }
                        else
                        {
                            int myedge=-1;
                            for(int e_j=0; e_j<pf2.edges.size(); e_j++)
                            {
                                if(pf2.point_is_on_edge(e_j,ipoint))
                                    myedge=e_j;
                            }
                            //edge through edge
                            if(myedge != -1)
                            {
                                //graph.points.push_back(ipoint);
                               // std::cout<<"ipoint: "<<ipoint.X<<","<<ipoint.Y<<","<<ipoint.Z<<"\n";

                                //graph.points.push_back(ipoint);
                                if(pf2.point_is_behind_edge(myedge,v0) &&
                                   pf2.point_is_behind_edge(myedge,v1))
                                    {
                                        //pf4.edges[e_i].topo_group=1;
                                        int new_v = pf4.get_point_or_add(ipoint);
                                        pf4.bisect_edge(e_i,new_v,1,1);

                                        new_v = pf5.get_point_or_add(ipoint);
                                        pf5.faces[face_j].addVertex(new_v);
                                    }
                                else if(!pf2.point_is_behind_edge(myedge,v0) &&
                                        !pf2.point_is_behind_edge(myedge,v1))
                                    {
                                        //pf4.edges[e_i].topo_group=0;
                                        int new_v = pf4.get_point_or_add(ipoint);
                                        pf4.bisect_edge(e_i,new_v,0,0);

                                        new_v = pf5.get_point_or_add(ipoint);
                                        pf5.faces[face_j].addVertex(new_v);
                                    }
                                else if(pf2.point_is_behind_edge(myedge,v0))
                                    {
                                        int new_v = pf4.get_point_or_add(ipoint);
                                        pf4.bisect_edge(e_i,new_v,1,0);

                                        new_v = pf5.get_point_or_add(ipoint);
                                        pf5.faces[face_j].addVertex(new_v);
                                    }
                                else
                                    {
                                        int new_v = pf4.get_point_or_add(ipoint);
                                        pf4.bisect_edge(e_i,new_v,0,1);

                                        new_v = pf5.get_point_or_add(ipoint);
                                        pf5.faces[face_j].addVertex(new_v);
                                    }
                            }
                            //edge through face
                            else if(pf2.point_is_in_front_of_face(face_j,v0))
                            {
                                //graph.points.push_back(ipoint);
                                int new_v = pf4.get_point_or_add(ipoint);
                                pf4.bisect_edge(e_i,new_v,0,1);

                                new_v = pf5.get_point_or_add(ipoint);
                                pf5.faces[face_j].addVertex(new_v);
                            }
                            else
                            {
                                //graph.points.push_back(ipoint);
                                int new_v = pf4.get_point_or_add(ipoint);
                                pf4.bisect_edge(e_i,new_v,1,0);

                                new_v = pf5.get_point_or_add(ipoint);
                                pf5.faces[face_j].addVertex(new_v);
                            }
                        }
                    }
                }
            }
        }
    }
    std::cout<<"...\n";

    if(false);
    for(int face_i=0;face_i<pf.faces.size();face_i++)
    {
        poly_face f=pf.faces[face_i];
        core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
        core::vector3df ipoint;

        for(int e_i=0; e_i<pf5.edges.size(); e_i++)
        {
            core::vector3df v0=pf5.getVertex(e_i,0).V;
            core::vector3df v1=pf5.getVertex(e_i,1).V;

            if(line_intersects_plane(f_plane,v0,v1,ipoint) && pf5.edges[e_i].topo_group != -1)
            {
                //graph.points.push_back(ipoint);
               if(pf.is_point_on_face(&f,ipoint))
                {
                     //graph.points.push_back(ipoint);
                    num++;
                    // std::cout<<"ipoint: "<<ipoint.X<<","<<ipoint.Y<<","<<ipoint.Z<<"\n";
                    // graph.lines.push_back(core::line3df(pf5.getVertex(e_i,1).V,pf5.getVertex(e_i,0).V));
                    if(pf2.find_point(ipoint) != -1)
                    {
                        //graph.points.push_back(ipoint);

                        int new_v = pf5.get_point_or_add(ipoint);
                        core::vector3df other_end;
                        if(new_v == pf5.edges[e_i].v0)
                        {
                            core::vector3df r = v1 - v0;
                            r.normalize();
                            other_end = v0+r;
                        }
                        else
                        {
                            core::vector3df r = v0 - v1;
                            r.normalize();
                            other_end = v1+r;
                        }
                        //point on point
                        int double_point=pf.find_point(ipoint);
                        if(double_point != -1)
                        {
                           // graph.points.push_back(ipoint);
                            if(pf.point_is_behind_point(double_point,other_end))
                                pf5.edges[e_i].topo_group=1;
                            else
                                pf5.edges[e_i].topo_group=0;
                        }
                        else
                        {
                            int myedge=-1;
                            for(int e_j=0; e_j<pf.edges.size(); e_j++)
                            {
                                if(pf.point_is_on_edge(e_j,ipoint))
                                    myedge=e_j;
                            }
                            //point on edge
                            if(myedge != -1)
                            {
                                //graph.points.push_back(ipoint);
                                if(pf.point_is_behind_edge(myedge,other_end))
                                    pf5.edges[e_i].topo_group=1;
                                else
                                    pf5.edges[e_i].topo_group=0;

                                new_v = pf4.get_point_or_add(ipoint);
                                pf4.faces[face_i].addVertex(new_v);
                            }
                            else    //point on face
                            {
                                //graph.points.push_back(ipoint);
                                if(pf.point_is_in_front_of_face(face_i,other_end))
                                    pf5.edges[e_i].topo_group=0;
                                else
                                    pf5.edges[e_i].topo_group=1;

                                new_v = pf4.get_point_or_add(ipoint);
                                pf4.faces[face_i].addVertex(new_v);
                            }
                        }
                    }
                    else
                    {
                        int double_point=pf.find_point(ipoint);

                        //edge through point
                        if(double_point != -1)
                        {
                            if(pf.point_is_behind_point(double_point,v0) &&
                               pf.point_is_behind_point(double_point,v1))
                                {
                                    pf5.edges[e_i].topo_group=1;
                                }
                            else if(!pf.point_is_behind_point(double_point,v0) &&
                                    !pf.point_is_behind_point(double_point,v1))
                                {
                                    pf5.edges[e_i].topo_group=0;
                                }
                            else if(pf.point_is_behind_point(double_point,v0))
                                {
                                    int new_v = pf5.get_point_or_add(ipoint);
                                    pf5.bisect_edge(e_i,new_v,1,0);
                                }
                            else
                                {
                                    int new_v = pf5.get_point_or_add(ipoint);
                                    pf5.bisect_edge(e_i,new_v,0,1);
                                }

                        }
                        else
                        {
                            int myedge=-1;
                            for(int e_j=0; e_j<pf.edges.size(); e_j++)
                            {
                                if(pf.point_is_on_edge(e_j,ipoint))
                                    myedge=e_j;
                            }
                            //edge through edge
                            if(myedge != -1)
                            {
                                //graph.points.push_back(ipoint);
                                if(pf.point_is_behind_edge(myedge,v0) &&
                                   pf.point_is_behind_edge(myedge,v1))
                                    {
                                        //graph.points.push_back(ipoint);
                                        //pf5.edges[e_i].topo_group=1;
                                        int new_v = pf5.get_point_or_add(ipoint);
                                        pf5.bisect_edge(e_i,new_v,1,1);

                                        new_v = pf4.get_point_or_add(ipoint);
                                        pf4.faces[face_i].addVertex(new_v);
                                    }
                                else if(!pf.point_is_behind_edge(myedge,v0) &&
                                        !pf.point_is_behind_edge(myedge,v1))
                                    {
                                        //graph.points.push_back(ipoint);
                                        //pf5.edges[e_i].topo_group=0;
                                        int new_v = pf5.get_point_or_add(ipoint);
                                        pf5.bisect_edge(e_i,new_v,0,0);

                                        new_v = pf4.get_point_or_add(ipoint);
                                        pf4.faces[face_i].addVertex(new_v);
                                    }
                                else if(pf.point_is_behind_edge(myedge,v0))
                                    {
                                        int new_v = pf5.get_point_or_add(ipoint);
                                        pf5.bisect_edge(e_i,new_v,1,0);

                                        new_v = pf4.get_point_or_add(ipoint);
                                        pf4.faces[face_i].addVertex(new_v);
                                    }
                                else
                                    {
                                        int new_v = pf5.get_point_or_add(ipoint);
                                        pf5.bisect_edge(e_i,new_v,0,1);

                                        new_v = pf4.get_point_or_add(ipoint);
                                        pf4.faces[face_i].addVertex(new_v);
                                    }
                            }
                            //edge through face
                            else if(pf.point_is_in_front_of_face(face_i,v0))
                            {
                                int new_v = pf5.get_point_or_add(ipoint);
                                pf5.bisect_edge(e_i,new_v,0,1);

                                new_v = pf4.get_point_or_add(ipoint);
                                pf4.faces[face_i].addVertex(new_v);
                            }
                            else
                            {
                                int new_v = pf5.get_point_or_add(ipoint);
                                pf5.bisect_edge(e_i,new_v,1,0);

                                new_v = pf4.get_point_or_add(ipoint);
                                pf4.faces[face_i].addVertex(new_v);
                            }
                        }
                    }
                }
            }
        }
    }
*/
/*
for(int f_i=0; f_i < pf4.faces.size(); f_i++)
    {

        pf4.faces[f_i].loops.clear();
        std::cout<<"face "<<f_i<<"\n";
        //std::cout<<pf4.faces[f_i].edges.size()<<"\n";

        std::vector<int> rules;
        rules.push_back(3);
        rules.push_back(5);
        pf4.calc_loops2(pf4.faces[f_i],rules);

        rules.clear();
        rules.push_back(!rule);
        rules.push_back(5);
        rules.push_back(3);
        pf4.calc_loops2(pf4.faces[f_i],rules);


        //for(poly_loop loop : pf4.faces[f_i].loops)
        if(false)
        for(int p_i = 0; p_i < pf4.faces[f_i].loops.size(); p_i++)
        {
            poly_loop loop = pf4.faces[f_i].loops[p_i];
            //std::cout<<"vol loop with "<<loop.vertices.size()<<"\n";

            //core::vector3df test_vec = pf4.get_test_vec(f_i,p_i);
            //graph.lines.push_back(core::line3df(test_vec,pf4.vertices[pf4.faces[f_i].loops[p_i].vertices[0]].V));

            //std::cout<<"testing\n";

            bool bCoplanar=false;
            if(false)
            for(int f_j = 0; f_j <pf5.faces.size(); f_j++)
            {
//                if(pf5.point_is_coplanar(f_j,test_vec))
//                    bCoplanar = true;
                    //std::cout<<"coplanar\n";
            }


            //if(bCoplanar)
            {
                std::vector<int> tevec;
                std::vector<int> tevec2;
                    for(int i = 0; i<loop.vertices.size()-1; i++)
                        tevec.push_back(pf4.find_edge(loop.vertices[i],loop.vertices[i+1]));
                    tevec.push_back(pf4.find_edge(loop.vertices[loop.vertices.size()-1],loop.vertices[0]));

                    for(int e_i : tevec)
                        if(pf4.edges[e_i].topo_group==5)
                        {
                            tevec2.push_back(e_i);
                            //removal.push_back(e_i);
                        }
                    tevec.clear();
                    for(int e_i : pf4.faces[f_i].edges)
                    {
                        bool b = false;
                        for(int e_j : tevec2)
                        {
                            if(e_i == e_j)
                                b=true;
                        }
                        if(!b)
                            tevec.push_back(e_i);
                    }
                    pf4.faces[f_i].edges=tevec;
            }
        }
        pf4.faces[f_i].loops.clear();
    }
*/

/*
    std::cout<<"testing loop search\n";
    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
    {
        pf4.faces[f_i].loops.clear();

        std::cout<<"face "<<f_i<<": "<<pf4.faces[f_i].edges.size()<<" edges\n";

        std::vector<int> rules;
        rules.push_back(3);
        rules.push_back(5);
        pf4.calc_loops2(pf4.faces[f_i],rules);
    }
    */

/*

polyfold clip_poly(polyfold& pf, polyfold& pf2, int rule, LineHolder &graph)
{
    polyfold pf4 = pf;
    polyfold pf5 = pf2;
    polyfold ret;

    std::cout<<"Creating new geometry...\n";

    for(poly_edge &edge : pf4.edges)
    {
     edge.topo_group=2;
    }

    for(poly_edge &edge : pf5.edges)
    {
     edge.topo_group=2;
    }
    int num=0;

    for(int face_j=0;face_j<pf2.faces.size();face_j++)
    {
        poly_face f2=pf2.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);
        core::vector3df ipoint;

        for(int e_i=0; e_i<pf4.edges.size(); e_i++)
        {
            core::vector3df v0=pf4.getVertex(e_i,0).V;
            core::vector3df v1=pf4.getVertex(e_i,1).V;
            if(line_intersects_plane(f2_plane,v0,v1,ipoint) && pf4.edges[e_i].topo_group != -1)
            {
                //if(face_j == 4)
                //    graph.points.push_back(ipoint);
                if(pf2.is_point_on_face(&f2,ipoint) )
                {
                   // if(face_j == 3)
                   // {
                   //  graph.points.push_back(ipoint);
                   //  graph.lines.push_back(core::line3df(pf4.getVertex(e_i,1).V,pf4.getVertex(e_i,0).V));
                   // }
                    num++;
                    // std::cout<<"ipoint: "<<ipoint.X<<","<<ipoint.Y<<","<<ipoint.Z<<"\n";
                    // graph.lines.push_back(core::line3df(pf4.getVertex(e_i,1).V,pf4.getVertex(e_i,0).V));
                    if(pf.find_point(ipoint) != -1)
                    {

                        int new_v = pf4.get_point_or_add(ipoint);
                        core::vector3df other_end;
                        if(new_v == pf4.edges[e_i].v0)
                        {
                            core::vector3df r = v1 - v0;
                            r.normalize();
                            other_end = v0+r;
                        }
                        else
                        {
                            core::vector3df r = v0 - v1;
                            r.normalize();
                            other_end = v1+r;
                        }
                        //point on point
                        int double_point=pf2.find_point(ipoint);
                        if(double_point != -1)
                        {

                           // graph.points.push_back(ipoint);
                            //std::cout<<e_i<<"\n";
                            //std::cout<<"ipoint: "<<ipoint.X<<","<<ipoint.Y<<","<<ipoint.Z<<"\n";
                            //std::cout<<"to: "<<other_end.X<<","<<other_end.Y<<","<<other_end.Z<<"\n";
                            if(pf2.point_is_behind_point(double_point,other_end))
                                pf4.edges[e_i].topo_group=1;
                            else
                                pf4.edges[e_i].topo_group=0;
                        }
                        else
                        {

                            int myedge=-1;
                            for(int e_j=0; e_j<pf2.edges.size(); e_j++)
                            {
                                if(pf2.point_is_on_edge(e_j,ipoint))
                                    myedge=e_j;
                            }
                            //point on edge
                            if(myedge != -1)
                            {
                                //graph.points.push_back(ipoint);
                                //graph.points.push_back(ipoint);
                                //std::cout<<"ipoint: "<<ipoint.X<<","<<ipoint.Y<<","<<ipoint.Z<<"\n";
                                //std::cout<<"to: "<<other_end.X<<","<<other_end.Y<<","<<other_end.Z<<"\n";
                                if(pf2.point_is_behind_edge(myedge,other_end))
                                    pf4.edges[e_i].topo_group=1;
                                else
                                    pf4.edges[e_i].topo_group=0;

                                new_v = pf5.get_point_or_add(ipoint);
                                pf5.faces[face_j].addVertex(new_v);

                            }
                            else    //point on face
                            {
                               // graph.points.push_back(ipoint);
                                //std::cout<<e_i<<" "<<face_j<<"\n";
                                //std::cout<<"ipoint: "<<ipoint.X<<","<<ipoint.Y<<","<<ipoint.Z<<"\n";
                                //std::cout<<"to: "<<other_end.X<<","<<other_end.Y<<","<<other_end.Z<<"\n";
                                if(pf2.point_is_in_front_of_face(face_j,other_end))
                                    pf4.edges[e_i].topo_group=0;
                                else
                                    pf4.edges[e_i].topo_group=1;

                                new_v = pf5.get_point_or_add(ipoint);
                                pf5.faces[face_j].addVertex(new_v);
                            }
                        }
                    }
                    else
                    {

                        int double_point=pf2.find_point(ipoint);

                        //edge through point
                        if(double_point != -1)
                        {
                            //graph.points.push_back(ipoint);
                            //std::cout<<e_i<<"...\n";
                            //std::cout<<pf2.point_is_behind_point(double_point,v0)<<"\n";
                            //std::cout<<pf2.point_is_behind_point(double_point,v1)<<"\n";

                            //int new_v = pf4.get_point_or_add(ipoint);
                            //pf4.faces[face_j].addVertex(new_v);

                            //graph.points.push_back(ipoint);

                            if(pf2.point_is_behind_point(double_point,v0) &&
                               pf2.point_is_behind_point(double_point,v1))
                                {
                                   // std::cout<<"ipoint: "<<ipoint.X<<","<<ipoint.Y<<","<<ipoint.Z<<"\n";
                                    //std::cout<<"v0: "<<v0.X<<","<<v0.Y<<","<<v0.Z<<"\n";
                                    //std::cout<<"v1: "<<v1.X<<","<<v1.Y<<","<<v1.Z<<"\n";

                                    //pf4.edges[e_i].topo_group=1;

                                    int new_v = pf4.get_point_or_add(ipoint);
                                    pf4.bisect_edge(e_i,new_v,1,1);

                                    new_v = pf5.get_point_or_add(ipoint);
                                    pf5.faces[face_j].addVertex(new_v);
                                }
                            else if(!pf2.point_is_behind_point(double_point,v0) &&
                                    !pf2.point_is_behind_point(double_point,v1))
                                {
                                    //pf4.edges[e_i].topo_group=0;

                                    int new_v = pf4.get_point_or_add(ipoint);
                                    pf4.bisect_edge(e_i,new_v,0,0);

                                    new_v = pf5.get_point_or_add(ipoint);
                                    pf5.faces[face_j].addVertex(new_v);
                                }
                            else if(pf2.point_is_behind_point(double_point,v0))
                                {
                                    int new_v = pf4.get_point_or_add(ipoint);
                                    pf4.bisect_edge(e_i,new_v,1,0);
                                   // graph.points.push_back(ipoint);
                                }
                            else
                                {
                                    int new_v = pf4.get_point_or_add(ipoint);
                                    pf4.bisect_edge(e_i,new_v,0,1);
                                    //graph.points.push_back(ipoint);
                                }
                        }
                        else
                        {
                            int myedge=-1;
                            for(int e_j=0; e_j<pf2.edges.size(); e_j++)
                            {
                                if(pf2.point_is_on_edge(e_j,ipoint))
                                    myedge=e_j;
                            }
                            //edge through edge
                            if(myedge != -1)
                            {
                                //graph.points.push_back(ipoint);
                               // std::cout<<"ipoint: "<<ipoint.X<<","<<ipoint.Y<<","<<ipoint.Z<<"\n";

                                //graph.points.push_back(ipoint);
                                if(pf2.point_is_behind_edge(myedge,v0) &&
                                   pf2.point_is_behind_edge(myedge,v1))
                                    {
                                        //pf4.edges[e_i].topo_group=1;
                                        int new_v = pf4.get_point_or_add(ipoint);
                                        pf4.bisect_edge(e_i,new_v,1,1);

                                        new_v = pf5.get_point_or_add(ipoint);
                                        pf5.faces[face_j].addVertex(new_v);
                                    }
                                else if(!pf2.point_is_behind_edge(myedge,v0) &&
                                        !pf2.point_is_behind_edge(myedge,v1))
                                    {
                                        //pf4.edges[e_i].topo_group=0;
                                        int new_v = pf4.get_point_or_add(ipoint);
                                        pf4.bisect_edge(e_i,new_v,0,0);

                                        new_v = pf5.get_point_or_add(ipoint);
                                        pf5.faces[face_j].addVertex(new_v);
                                    }
                                else if(pf2.point_is_behind_edge(myedge,v0))
                                    {
                                        int new_v = pf4.get_point_or_add(ipoint);
                                        pf4.bisect_edge(e_i,new_v,1,0);

                                        new_v = pf5.get_point_or_add(ipoint);
                                        pf5.faces[face_j].addVertex(new_v);
                                    }
                                else
                                    {
                                        int new_v = pf4.get_point_or_add(ipoint);
                                        pf4.bisect_edge(e_i,new_v,0,1);

                                        new_v = pf5.get_point_or_add(ipoint);
                                        pf5.faces[face_j].addVertex(new_v);
                                    }
                            }
                            //edge through face
                            else if(pf2.point_is_in_front_of_face(face_j,v0))
                            {
                                int new_v = pf4.get_point_or_add(ipoint);
                                pf4.bisect_edge(e_i,new_v,0,1);

                                new_v = pf5.get_point_or_add(ipoint);
                                pf5.faces[face_j].addVertex(new_v);
                            }
                            else
                            {
                                int new_v = pf4.get_point_or_add(ipoint);
                                pf4.bisect_edge(e_i,new_v,1,0);

                                new_v = pf5.get_point_or_add(ipoint);
                                pf5.faces[face_j].addVertex(new_v);
                            }
                        }
                    }
                }
            }
        }
    }
    std::cout<<"...\n";

    for(int face_i=0;face_i<pf.faces.size();face_i++)
    {
        poly_face f=pf.faces[face_i];
        core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
        core::vector3df ipoint;

        for(int e_i=0; e_i<pf5.edges.size(); e_i++)
        {
            core::vector3df v0=pf5.getVertex(e_i,0).V;
            core::vector3df v1=pf5.getVertex(e_i,1).V;

            if(line_intersects_plane(f_plane,v0,v1,ipoint) && pf5.edges[e_i].topo_group != -1)
            {
                //graph.points.push_back(ipoint);
               if(pf.is_point_on_face(&f,ipoint))
                {
                     //graph.points.push_back(ipoint);
                    num++;
                    // std::cout<<"ipoint: "<<ipoint.X<<","<<ipoint.Y<<","<<ipoint.Z<<"\n";
                    // graph.lines.push_back(core::line3df(pf5.getVertex(e_i,1).V,pf5.getVertex(e_i,0).V));
                    if(pf2.find_point(ipoint) != -1)
                    {
                        //graph.points.push_back(ipoint);

                        int new_v = pf5.get_point_or_add(ipoint);
                        core::vector3df other_end;
                        if(new_v == pf5.edges[e_i].v0)
                        {
                            core::vector3df r = v1 - v0;
                            r.normalize();
                            other_end = v0+r;
                        }
                        else
                        {
                            core::vector3df r = v0 - v1;
                            r.normalize();
                            other_end = v1+r;
                        }
                        //point on point
                        int double_point=pf.find_point(ipoint);
                        if(double_point != -1)
                        {
                           // graph.points.push_back(ipoint);
                            if(pf.point_is_behind_point(double_point,other_end))
                                pf5.edges[e_i].topo_group=1;
                            else
                                pf5.edges[e_i].topo_group=0;
                        }
                        else
                        {
                            int myedge=-1;
                            for(int e_j=0; e_j<pf.edges.size(); e_j++)
                            {
                                if(pf.point_is_on_edge(e_j,ipoint))
                                    myedge=e_j;
                            }
                            //point on edge
                            if(myedge != -1)
                            {
                                //graph.points.push_back(ipoint);
                                if(pf.point_is_behind_edge(myedge,other_end))
                                    pf5.edges[e_i].topo_group=1;
                                else
                                    pf5.edges[e_i].topo_group=0;

                                new_v = pf4.get_point_or_add(ipoint);
                                pf4.faces[face_i].addVertex(new_v);
                            }
                            else    //point on face
                            {
                                //graph.points.push_back(ipoint);
                                if(pf.point_is_in_front_of_face(face_i,other_end))
                                    pf5.edges[e_i].topo_group=0;
                                else
                                    pf5.edges[e_i].topo_group=1;

                                new_v = pf4.get_point_or_add(ipoint);
                                pf4.faces[face_i].addVertex(new_v);
                            }
                        }
                    }
                    else
                    {
                        int double_point=pf.find_point(ipoint);

                        //edge through point
                        if(double_point != -1)
                        {
                            if(pf.point_is_behind_point(double_point,v0) &&
                               pf.point_is_behind_point(double_point,v1))
                                {
                                    pf5.edges[e_i].topo_group=1;
                                }
                            else if(!pf.point_is_behind_point(double_point,v0) &&
                                    !pf.point_is_behind_point(double_point,v1))
                                {
                                    pf5.edges[e_i].topo_group=0;
                                }
                            else if(pf.point_is_behind_point(double_point,v0))
                                {
                                    int new_v = pf5.get_point_or_add(ipoint);
                                    pf5.bisect_edge(e_i,new_v,1,0);
                                }
                            else
                                {
                                    int new_v = pf5.get_point_or_add(ipoint);
                                    pf5.bisect_edge(e_i,new_v,0,1);
                                }

                        }
                        else
                        {
                            int myedge=-1;
                            for(int e_j=0; e_j<pf.edges.size(); e_j++)
                            {
                                if(pf.point_is_on_edge(e_j,ipoint))
                                    myedge=e_j;
                            }
                            //edge through edge
                            if(myedge != -1)
                            {
                                //graph.points.push_back(ipoint);
                                if(pf.point_is_behind_edge(myedge,v0) &&
                                   pf.point_is_behind_edge(myedge,v1))
                                    {
                                        //graph.points.push_back(ipoint);
                                        //pf5.edges[e_i].topo_group=1;
                                        int new_v = pf5.get_point_or_add(ipoint);
                                        pf5.bisect_edge(e_i,new_v,1,1);

                                        new_v = pf4.get_point_or_add(ipoint);
                                        pf4.faces[face_i].addVertex(new_v);
                                    }
                                else if(!pf.point_is_behind_edge(myedge,v0) &&
                                        !pf.point_is_behind_edge(myedge,v1))
                                    {
                                        //graph.points.push_back(ipoint);
                                        //pf5.edges[e_i].topo_group=0;
                                        int new_v = pf5.get_point_or_add(ipoint);
                                        pf5.bisect_edge(e_i,new_v,0,0);

                                        new_v = pf4.get_point_or_add(ipoint);
                                        pf4.faces[face_i].addVertex(new_v);
                                    }
                                else if(pf.point_is_behind_edge(myedge,v0))
                                    {
                                        int new_v = pf5.get_point_or_add(ipoint);
                                        pf5.bisect_edge(e_i,new_v,1,0);

                                        new_v = pf4.get_point_or_add(ipoint);
                                        pf4.faces[face_i].addVertex(new_v);
                                    }
                                else
                                    {
                                        int new_v = pf5.get_point_or_add(ipoint);
                                        pf5.bisect_edge(e_i,new_v,0,1);

                                        new_v = pf4.get_point_or_add(ipoint);
                                        pf4.faces[face_i].addVertex(new_v);
                                    }
                            }
                            //edge through face
                            else if(pf.point_is_in_front_of_face(face_i,v0))
                            {
                                int new_v = pf5.get_point_or_add(ipoint);
                                pf5.bisect_edge(e_i,new_v,0,1);

                                new_v = pf4.get_point_or_add(ipoint);
                                pf4.faces[face_i].addVertex(new_v);
                            }
                            else
                            {
                                int new_v = pf5.get_point_or_add(ipoint);
                                pf5.bisect_edge(e_i,new_v,1,0);

                                new_v = pf4.get_point_or_add(ipoint);
                                pf4.faces[face_i].addVertex(new_v);
                            }
                        }
                    }
                }
            }
        }
    }

    //std::cout<<"found "<<num<<" intersection points\n";
    num=0;


    num=0;
    int num2=0;
    for(int face_i=0;face_i<pf.faces.size();face_i++)
        for(int face_j=0;face_j<pf2.faces.size();face_j++)
        {
            poly_face f=pf.faces[face_i];
            poly_face f2=pf2.faces[face_j];

            plane res;

            core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
            core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);


            if(f_plane.getIntersectionWithPlane(f2_plane,res.point,res.norm))
            {
                polyfold verts;
                for(int v_i : pf4.faces[face_i].vertices)
                {
                    core::vector3df v = pf4.vertices[v_i].V;
                    if(abs(f2_plane.getDistanceTo(v))<0.1 && pf2.is_point_on_face(&f2,v))
                    {
                        verts.get_point_or_add(v);
                        num++;
                    }
                }

                for(int v_j : pf5.faces[face_j].vertices)
                 {
                    core::vector3df v = pf5.vertices[v_j].V;
                    if(abs(f_plane.getDistanceTo(v))<0.1 && pf.is_point_on_face(&f,v))
                    {
                        verts.get_point_or_add(v);
                        num2++;
                    }
                 }
                 if(verts.vertices.size()>0)
                 {
                     sort_inline_vertices(verts);
                     for(int i=0;i<verts.vertices.size()-1;i++)
                     {

                        //=======
                        core::vector3df r = verts.vertices[i].V + (verts.vertices[i+1].V - verts.vertices[i].V)*0.5;
                        //graph.points.push_back(verts.vertices[i].V);

                        //if(pf.is_point_on_face(&f,r) && pf2.is_point_on_face(&f2,r))
                        {
                            //graph.points.push_back(verts.vertices[i].V);
                            //graph.points.push_back(verts.vertices[i+1].V);

                            int v0 = pf4.get_point_or_add(verts.vertices[i].V);
                            int v1 = pf4.get_point_or_add(verts.vertices[i+1].V);

                            int new_e = pf4.find_edge(v0,v1);
                            if(new_e != -1 && pf4.edges[new_e].topo_group != 3)
                            {
                                //graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));
                                pf4.edges[new_e].topo_group=5;
                                //std::cout<<new_e<<"\n";
                            }
                            else
                            {
                                new_e = pf4.get_edge_or_add(v0,v1,3);
                                //std::cout<<new_e<<"\n";
                                //graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));
                            }

                            pf4.faces[face_i].addEdge(new_e);
                            //i++;
                        }
                        //graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));
                     }
                 }
            }
        }

    poly_face f;
    num=0;

    //========================================
    pf4.apply_topology_groups(rule);

    //if(false)
    {


    std::vector<int> removal;
    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
    {
        pf4.faces[f_i].loops.clear();

        //std::cout<<pf4.faces[f_i].edges.size()<<"\n";

        std::vector<int> rules;
        rules.push_back(3);
        rules.push_back(5);
        pf4.calc_loops2(pf4.faces[f_i],rules);

        rules.clear();
        rules.push_back(!rule);
        rules.push_back(5);
        rules.push_back(3);
        pf4.calc_loops2(pf4.faces[f_i],rules);

        for(poly_loop loop : pf4.faces[f_i].loops)
        {
            //std::cout<<"vol loop with "<<loop.vertices.size()<<"\n";
            std::vector<int> tevec;
            std::vector<int> tevec2;

            //if(!keep)
            {
                for(int i = 0; i<loop.vertices.size()-1; i++)
                    tevec.push_back(pf4.find_edge(loop.vertices[i],loop.vertices[i+1]));
                tevec.push_back(pf4.find_edge(loop.vertices[loop.vertices.size()-1],loop.vertices[0]));

                for(int e_i : tevec)
                    if(pf4.edges[e_i].topo_group==5)
                    {
                        tevec2.push_back(e_i);
                        //removal.push_back(e_i);
                    }
                tevec.clear();
                for(int e_i : pf4.faces[f_i].edges)
                {
                    bool b = false;
                    for(int e_j : tevec2)
                    {
                        if(e_i == e_j)
                            b=true;
                    }
                    if(!b)
                        tevec.push_back(e_i);
                }
                pf4.faces[f_i].edges=tevec;
            }
        }
        pf4.faces[f_i].loops.clear();
    }

    for(int e_i = 0; e_i <pf4.edges.size(); e_i++)
        {
            if(pf4.edges[e_i].topo_group==5)
            {
                pf4.edges[e_i].topo_group=3;
                //for(int e_j : removal)
                //    if(e_i == e_j)
                //        pf4.edges[e_i].topo_group=-1;
            }
        }


    //===============================

    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
    {

        f.clear();
        num=0;
        //std::cout<<pf4.faces[f_i].edges.size()<<" edges \n ";
        //for(int e_i : pf4.faces[f_i].edges)
       //     std::cout<<pf4.edges[e_i].topo_group<<" ";
        //std::cout<<"\n";
        pf4.faces[f_i].loops.clear();

        std::vector<int> rules;
        rules.push_back(rule);
        rules.push_back(3);
        pf4.calc_loops2(pf4.faces[f_i],rules);
        pf4.sort_loops(pf4.faces[f_i]);


        std::cout<<"face "<<f_i<<": "<<pf4.faces[f_i].loops.size()<<" loops ";
        for(poly_loop loop: pf4.faces[f_i].loops)
            std::cout<<loop.vertices.size()<<" ";
        std::cout<<"\n";

        f.m_normal = pf4.faces[f_i].m_normal;
        f.bFlippedNormal = pf4.faces[f_i].bFlippedNormal;
        //f.m_center = pf4.faces[f_i].m_center;

        for(poly_loop loop : pf4.faces[f_i].loops)
        {
            poly_loop new_loop;
            for(int v_i : loop.vertices)
            {
                int v = ret.get_point_or_add(pf4.vertices[v_i].V);
                new_loop.vertices.push_back(v);
                f.addVertex(v);
            }

            std::vector<int> tempv = loop.vertices;
            tempv.push_back(tempv[0]);

            for(int i=0; i < tempv.size()-1; i++)
               {
                core::vector3df v0=pf4.vertices[tempv[i]].V;
                core::vector3df v1=pf4.vertices[tempv[i+1]].V;
                int new_e = ret.get_edge_or_add(ret.get_point_or_add(v0),ret.get_point_or_add(v1),0);
                f.addEdge(new_e);
               }

            new_loop.type = loop.type;
            ret.calc_loop_bbox(f,new_loop);
            f.loops.push_back(new_loop);
        }

        ret.calc_center(f);
        if(f.edges.size()>2 && f.loops.size() > 0)
        {
            ret.sort_loops(f);
            ret.faces.push_back(f);
        }
    }


    //ret.recalc_bbox_and_loops();

    ret.bbox.reset(ret.vertices[0].V);
    for(poly_vert v : ret.vertices)
        ret.bbox.addInternalPoint(v.V);

    ret.topology = pf4.topology;
    }



    //return pf4;
    return ret;
}
*/

/*

bool polyfold::point_is_behind_point(int point, core::vector3df v )
{

    std::vector<int> convex_edges;
    std::vector<int> concave_edges;

    for(int e_i =0; e_i< this->edges.size(); e_i++)
    {
        if(this->edges[e_i].v0 == point || this->edges[e_i].v1 == point)
        {
            if(this->classify_edge(e_i)==TOP_CONVEX)
                convex_edges.push_back(e_i);
            else
                concave_edges.push_back(e_i);
        }
    }

    bool bIsBehindConvex = true;

    for(int e_i : convex_edges)
        if(this->point_is_behind_edge(e_i,v)==false)
            bIsBehindConvex = false;

    bool bIsBehindConcave = true;
    if(concave_edges.size() > 0)
    {
        bIsBehindConcave = false;
        for(int e_i : concave_edges)
            if(this->point_is_behind_edge(e_i,v)==true)
               bIsBehindConcave = true;
    }

    return(bIsBehindConvex && bIsBehindConcave);
}
*/
/*


bool polyfold::point_is_in_front_of_face(int face, core::vector3df v )
{
    core::plane3df plane = core::plane3df(this->faces[face].m_center,this->faces[face].m_normal);
    {
        //std::cout<<plane.getDistanceTo(v)<<".\n";
        if(plane.getDistanceTo(v)<0.01)
            return false;
       // else if(plane.classifyPointRelation(v)==core::ISREL3D_FRONT)
       //     return true;
        else return true;
    }
}

bool polyfold::point_is_behind_face(int face, core::vector3df v )
{
    core::plane3df plane = core::plane3df(this->faces[face].m_center,this->faces[face].m_normal);
    {
        //std::cout<<plane.getDistanceTo(v)<<".\n";
        if(plane.getDistanceTo(v)>-0.01)
            return false;
       // else if(plane.classifyPointRelation(v)==core::ISREL3D_FRONT)
       //     return true;
        else return true;
    }

}
*/

/*

bool polyfold::point_is_behind_edge(int edge, core::vector3df v )
{
    if(this->classify_edge(edge)==TOP_CONVEX)
   {
       //std::cout<<"convex\n";
        for(int f_i=0; f_i<this->faces.size(); f_i++)
        {
            for(int e_i : this->faces[f_i].edges)
            {
                if(e_i == edge && this->point_is_in_front_of_face(f_i,v)==true)
                    return false;
            }
        }
        return true;
   }
   else // if(this->classify_edge(edge)==TOP_CONCAVE)
   {
      // std::cout<<"concave\n";
       for(int f_i=0; f_i<this->faces.size(); f_i++)
        {
            for(int e_i : this->faces[f_i].edges)
            {
                if(e_i == edge && this->point_is_in_front_of_face(f_i,v)==false)
                    return true;
            }
        }
        return false;
   }
}
*/

