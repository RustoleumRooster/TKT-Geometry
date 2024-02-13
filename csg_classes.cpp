
#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"
#include "tolerances.h"
#include "BVH.h"

using namespace irr;

#define LEFT_RIGHT_TEST_SMALL_NUMBER 0.005
#define LEFT_RIGHT_TEST_INCLUDE_NUMBER 0.01


core::vector3df poly_edge::position(const poly_vert* verts) const
{
    return  verts[v0].V + (verts[v1].V - verts[v0].V) * 0.5;
}

core::matrix4 poly_face::get2Dmat() const
{
    core::matrix4 R;

    R.buildRotateFromTo(this->getOrientingNormal(), core::vector3df(0, 1, 0));
    return R;
}

core::vector3df poly_face::getOrientingNormal() const
{
    /*
    if (is_parallel_normal(this->m_normal, core::vector3df(0, 1, 0)))
        return core::vector3df(0, 1, 0);
    else if (bFlippedNormal)
        return this->m_normal * -1;
    else
        return this->m_normal;*/
    
    if (m_normal.Y < 0)
        return m_normal * -1;
    else
        return m_normal;

    /*
    if (m_normal.Y < 0 && !bFlippedNormal)
        return m_normal * -1;
    else if (m_normal.Y < 0 && bFlippedNormal)
        return m_normal * -1;
    else
        return m_normal;*/
}

void polyfold::rotate(core::matrix4 MAT)
{
    for(poly_vert & vert: vertices)
    {
        MAT.rotateVect(vert.V);
    }
    for(poly_face & face: faces)
    {
        MAT.rotateVect(face.m_normal);
        face.m_normal.normalize();
        MAT.rotateVect(face.m_center);
        MAT.rotateVect(face.uv_origin);
    }
    for(surface_group& sg: surface_groups)
    {
        MAT.rotateVect(sg.point);
        MAT.rotateVect(sg.vec);
        sg.vec.normalize();
        MAT.rotateVect(sg.vec1);
        sg.vec1.normalize();
    }

    recalc_bbox();

    for(int f_i=0;f_i<this->faces.size();f_i++)
        for(int p_i=0;p_i<this->faces[f_i].loops.size();p_i++)
        {
            this->calc_loop_bbox(f_i,p_i);

          //  if(faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                set_loop_solid(f_i,p_i);
          //  else if(faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
         //       set_loop_open(f_i,p_i);
         //   else std::cout<<f_i<<" "<<p_i<<" undefined "<<faces[f_i].loops[p_i].topo_group<<"\n";
        }
}

void polyfold::translate(core::matrix4 MAT)
{
    for(poly_vert & vert: vertices)
    {
        MAT.translateVect(vert.V);
    }
    for(poly_face & face: faces)
    {
        MAT.translateVect(face.m_center);
        MAT.translateVect(face.uv_origin);
    }
    for(surface_group& sg: surface_groups)
    {
        MAT.translateVect(sg.point);
    }
    recalc_bbox();

    for(int f_i=0;f_i<this->faces.size();f_i++)
        for(int p_i=0;p_i<this->faces[f_i].loops.size();p_i++)
        {
            this->calc_loop_bbox(f_i,p_i);
        }
}

void polyfold::make_convex()
    {
        if(topology == TOP_CONCAVE)
        {
            for (int f_i = 0; f_i < faces.size(); f_i++)
            {
                faces[f_i].flip_normal();

                for (int p_i = 0; p_i < faces[f_i].loops.size(); p_i++)
                    set_loop_solid(f_i, p_i);
            }
            topology = TOP_CONVEX;
        }
        else if(topology == TOP_CONVEX)
        {
            //do nothing
        }
        else
        {
            //rarely/never used
            std::cout<<"... calculating topology\n";
            topology=TOP_CONVEX;
            recalc_faces();
        }
    }

void polyfold::make_concave()
    {
        if(topology == TOP_CONVEX)
        {
            for (int f_i=0;f_i<faces.size();f_i++)
            {
                faces[f_i].flip_normal();

                for(int p_i=0;p_i<faces[f_i].loops.size();p_i++)
                    set_loop_solid(f_i,p_i);
            }
            topology = TOP_CONCAVE;
        }
        else if(topology == TOP_CONCAVE)
        {
            //do nothing
        }
        else
        {
            //rarely/never used
            std::cout<<"... calculating topology\n";
            topology=TOP_CONCAVE;
            recalc_faces();
        }
    }

//rarely/never used
void polyfold::recalc_faces()
{
    std::cout << "recalc faces\n";
    recalc_bbox();

    for(int f_i=0;f_i<this->faces.size();f_i++)
    {
        calc_normal(f_i);

        for(int p_i=0;p_i<this->faces[f_i].loops.size();p_i++)
        {
            this->calc_loop_bbox(f_i,p_i);

          //  if(faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                set_loop_solid(f_i,p_i);
           // else if(faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
           //     set_loop_open(f_i,p_i);
          //  else std::cout<<"* warning * loop "<<f_i<<" "<<p_i<<" undefined "<<faces[f_i].loops[p_i].topo_group<<"\n";
        }
    }

    LineHolder nograph;
    if(topology==TOP_CONVEX)
    {
        make_convex2(nograph);
    }
    else
    {
        make_convex2(nograph);
        for(poly_face& f : faces)
            f.flip_normal();
        topology=TOP_CONCAVE;
    }
}

void polyfold::make_convex2(LineHolder& graph)
{
    LineHolder nograph;

    for(int f_i=0; f_i<faces.size(); f_i++)
    {
        if(faces[f_i].loops.size()==0)
            continue;

        polyfold verts;
        //graph.lines.push_back(core::line3df(v0,this->vertices[w].V));

        for(int e_0=0;e_0<faces[f_i].edges.size();e_0++)
        {
            core::vector3df axis = faces[f_i].m_normal;

            int e_i = faces[f_i].edges[e_0];

            core::vector3df v0;
            get_facial_point(f_i,e_i, get_edge_loop_no(f_i,e_i),v0,nograph);

            verts.get_point_or_add(v0);

            for(int f_j=0; f_j<this->faces.size(); f_j++)
            {
                if(f_i==f_j)
                    continue;

                poly_face f=this->faces[f_j];
                core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);

                core::vector3df ipoint;

                if(f_plane.getIntersectionWithLine(v0,axis,ipoint) && this->is_point_on_face(f_j,ipoint))
                {
                    verts.get_point_or_add(ipoint);
                    //graph.points.push_back(ipoint);
                }
            }
            //std::cout<<"ipoints: "<<verts.vertices.size()<<"\n";

            if(verts.vertices.size()%2==0)
            {
                sort_inline_vertices(verts);
                core::vector3df N = verts.vertices[0].V - verts.vertices[verts.vertices.size()-1].V;
                N.normalize();

                for(int i=0; i<verts.vertices.size(); i++)
                {
                    if(is_same_point(v0,verts.vertices[i].V))
                    {
                        if(faces[f_i].m_normal.dotProduct(N) < 0)
                        {
                            faces[f_i].flip_normal();
                        }
                        goto next_face;
                    }
                    N*=(-1);
                }
            }
            else
                graph.points.push_back(v0);
        }
        next_face:
        int z=0;
    }

    topology=TOP_CONVEX;
}

void polyfold::draw(video::IVideoDriver* driver, const video::SMaterial material, bool normals)
{
    driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    driver->setMaterial(material);
    for(int i =0; i<edges.size(); i++)
    {
        if(edges[i].topo_group != -1)
            driver->draw3DLine(vertices[edges[i].v0].V,vertices[edges[i].v1].V,video::SColor(12,12,0,48));
    }

    if(normals)
        for(poly_face face : this->faces)
        {
            driver->draw3DLine(face.m_center,face.m_center+face.m_normal*16,video::SColor(255,255,255,255));

        }
}

void polyfold::calc_center(poly_face& face)
{
    f32 xsum=0;
    f32 ysum=0;
    f32 zsum=0;

    for(int i=0; i < face.vertices.size(); i++)
    {
        xsum+=vertices[face.vertices[i]].V.X;
        ysum+=vertices[face.vertices[i]].V.Y;
        zsum+=vertices[face.vertices[i]].V.Z;
    }

    xsum/=face.vertices.size();
    ysum/=face.vertices.size();
    zsum/=face.vertices.size();

    face.m_center = core::vector3df(xsum,ysum,zsum);
}

void polyfold::calc_center(int f_i)
{
    f32 xsum=0;
    f32 ysum=0;
    f32 zsum=0;

    for(int i=0; i < this->faces[f_i].vertices.size(); i++)
    {
        xsum+=vertices[this->faces[f_i].vertices[i]].V.X;
        ysum+=vertices[this->faces[f_i].vertices[i]].V.Y;
        zsum+=vertices[this->faces[f_i].vertices[i]].V.Z;
    }

    xsum/=this->faces[f_i].vertices.size();
    ysum/=this->faces[f_i].vertices.size();
    zsum/=this->faces[f_i].vertices.size();

    this->faces[f_i].m_center = core::vector3df(xsum,ysum,zsum);
}

void polyfold::calc_normal(int f_i)
{
    this->calc_center(f_i);

    if(this->faces[f_i].vertices.size()>2)
        {
            int v_i0 = this->faces[f_i].vertices[0];
            int v_i1 = this->faces[f_i].vertices[1];
            int v_i2 = this->faces[f_i].vertices[2];

            core::vector3df v1 = vertices[v_i0].V - vertices[v_i1].V;
            core::vector3df v2 = vertices[v_i1].V - vertices[v_i2].V;
            core::vector3df cross = v1.crossProduct(v2);
            cross.normalize();

            this->faces[f_i].m_normal = cross;
            //this->faces[f_i].bFlippedNormal = false;
        }
}

void polyfold::calc_loop_bbox(int f_i, int p_i)
{
    core::matrix4 R = this->faces[f_i].get2Dmat();
    core::vector3df r;

    poly_loop *loop = &this->faces[f_i].loops[p_i];

    R.rotateVect(r,this->vertices[loop->vertices[0]].V);

    loop->min_x=r.X;
    loop->max_x=r.X;
    loop->min_z=r.Z;
    loop->max_z=r.Z;

    for(int v_i : loop->vertices)
        {
            R.rotateVect(r,this->vertices[v_i].V);
            loop->min_x=std::min(loop->min_x,r.X);
            loop->max_x=std::max(loop->max_x,r.X);
            loop->min_z=std::min(loop->min_z,r.Z);
            loop->max_z=std::max(loop->max_z,r.Z);
        }
}


void polyfold::calc_loop_bbox(poly_face& face, poly_loop &loop)
{
    core::matrix4 R = face.get2Dmat();
    core::vector3df r;


    R.rotateVect(r,this->vertices[loop.vertices[0]].V);

    loop.min_x=r.X;
    loop.max_x=r.X;
    loop.min_z=r.Z;
    loop.max_z=r.Z;

    for(int v_i : loop.vertices)
    {
        R.rotateVect(r,this->vertices[v_i].V);
        loop.min_x=std::min(loop.min_x,r.X);
        loop.max_x=std::max(loop.max_x,r.X);
        loop.min_z=std::min(loop.min_z,r.Z);
        loop.max_z=std::max(loop.max_z,r.Z);
    }
}

bool polyfold::is_closed_loop(int f_i, int p_i) const
{
    return faces[f_i].loops[p_i].depth % 2 == 0;
    /*
    poly_loop* loop = &this->faces[f_i].loops[p_i];

    if (loop->direction == LOOP_DIRECTION_SOLID)
    {
        return true;
    }
    else if (loop->direction == LOOP_DIRECTION_OPEN)
    {
        return false;
    }
    else
    {
        f32 testz = (this->faces[f_i].loops[p_i].max_z - this->faces[f_i].loops[p_i].min_z) * 0.5 + this->faces[f_i].loops[p_i].min_z;
        f32 testx = this->faces[f_i].loops[p_i].min_x - 10;
        int res = this->left_right_test(f_i, p_i, testx, testz);
        if (res == 0)
        {
            loop->direction = LOOP_DIRECTION_SOLID;
            return true;
        }
        loop->direction = LOOP_DIRECTION_OPEN;
        return false;
    }*/
}

void polyfold::set_loop_solid(int f_i, int p_i)
{
    poly_loop* loop = &this->faces[f_i].loops[p_i];
    bool is_solid;

    if(loop->vertices.size() == 0)
        return;

   // if (loop->direction == LOOP_DIRECTION_UNDEF)
    if(true)
    {
        this->calc_loop_bbox(f_i, p_i);

        f32 testz = (loop->max_z - loop->min_z) * 0.5 + loop->min_z;
        f32 testx = loop->min_x - 10;
        int res = this->left_right_test(f_i, p_i, testx, testz);

        is_solid = res == 0;
    }
    else
    {
        is_solid = loop->direction == LOOP_DIRECTION_SOLID;
    }

    if(!is_solid)
    {
        loop->reverse();
    }

    loop->direction = LOOP_DIRECTION_SOLID;
}

void polyfold::set_loop_open(int f_i, int p_i)
{
    poly_loop* loop = &this->faces[f_i].loops[p_i];
    bool is_open;

    if (loop->vertices.size() == 0)
        return;

    //if(loop->direction == LOOP_DIRECTION_UNDEF)
    if(true)
    {
        this->calc_loop_bbox(f_i, p_i);

        f32 testz = (loop->max_z - loop->min_z) * 0.5 + loop->min_z;
        f32 testx = loop->min_x - 10;
        int res = this->left_right_test(f_i, p_i, testx, testz);

        is_open = res == 1;
    }
    else 
    {
        is_open = loop->direction == LOOP_DIRECTION_OPEN;
    }

    if(!is_open)
    {
        loop->reverse();
    }

    loop->direction = LOOP_DIRECTION_OPEN;
}

void polyfold::recalc_bbox()
{
    if(this->vertices.size() > 0)
    {
    this->bbox.reset(this->vertices[0].V);
    for(poly_vert v : this->vertices)
        this->bbox.addInternalPoint(v.V);
    }
    else
        this->bbox.reset(core::vector3df(0,0,0));
}

//only called when creating primitives
void polyfold::recalc_bbox_and_loops()
{
    for(int f_i=0;f_i<this->faces.size();f_i++)
    {
        LineHolder nograph;
        this->calc_loops(f_i,nograph);

        //this->sort_loops(f_i);
        for(int p_i = 0; p_i< this->faces[f_i].loops.size(); p_i++)
        {
            if(this->faces[f_i].loops[p_i].type == LOOP_INNER)
                this->faces[f_i].loops[p_i].vertices.clear();
            else
                this->calc_loop_bbox(f_i,p_i);

           // this->faces[f_i].loops[p_i].direction = LOOP_DIRECTION_UNDEF;
            this->set_loop_solid(f_i, p_i);
        }
    }

    this->reduce_edges_vertices();

    if(this->vertices.size()>0)
    {
        this->bbox.reset(this->vertices[0].V);
        for(poly_vert v : this->vertices)
            this->bbox.addInternalPoint(v.V);
    }
}

int polyfold::get_point_or_add(core::vector3df point)
{
    for( int i=0; i<this->vertices.size(); i++)
    {
        poly_vert vert=this->vertices[i];
        if(is_same_point(vert.V,point))
           {
            return i;
           }
    }
    this->vertices.push_back(poly_vert(point.X,point.Y,point.Z));
    return this->vertices.size()-1;
}

int polyfold::find_point(core::vector3df point) const
{
    for( int i=0; i<this->vertices.size(); i++)
    {
        poly_vert vert=this->vertices[i];
        if(is_same_point(vert.V,point))
           {
            return i;
           }
    }
    return -1;
}

int polyfold::find_edge_accelerated(int v0, int v1, std::vector<u16>& hits) const
{
    aabb_struct aabb;

    vertices[v0].grow(&aabb);

    hits.clear();

    edges_BVH.intersect(aabb, hits);

    for (u16 e_i : hits)
    {
        if ((edges[e_i].v0 == v0 && edges[e_i].v1 == v1) || (edges[e_i].v1 == v0 && edges[e_i].v0 == v1))
        {
            return e_i;
        }
    }

    return -1;
}

int polyfold::find_edge_accelerated(core::vector3df v0, core::vector3df v1, std::vector<u16>& hits) const
{
    aabb_struct aabb;

    poly_vert(v0).grow(&aabb);

    hits.clear();

    edges_BVH.intersect(aabb, hits);

    for (u16 e_i : hits)
    {
        if ((getVertex(e_i,0).V == v0 && getVertex(e_i, 1).V == v1) || (getVertex(e_i, 1).V == v0 && getVertex(e_i, 0).V == v1))
        {
            return e_i;
        }
    }

    return -1;
}

int polyfold::find_point_accelerated(core::vector3df v) const
{
    return vertices_BVH.find_vertex_nocheck(v);
}

bool polyfold::is_point_in_loop(int f_i, int p_i, core::vector3df v) const
{
    core::matrix4 R = this->faces[f_i].get2Dmat();
    core::vector3df V = v;
    R.rotateVect(V);
    f32 small_number = LOOP_BBOX_SMALL_NUMBER;
    if(V.X < this->faces[f_i].loops[p_i].min_x - small_number || V.X > this->faces[f_i].loops[p_i].max_x + small_number ||
        V.Z < this->faces[f_i].loops[p_i].min_z - small_number || V.Z > this->faces[f_i].loops[p_i].max_z + small_number)
        {
        return false;
        }

    return (this->left_right_test(f_i, p_i, V.X,V.Z)==1);
}

bool polyfold::is_point_on_face(int f_i, core::vector3df v) const
{
    int n=0;

    for(int p_i=0; p_i< this->faces[f_i].loops.size(); p_i++)
    {
        if(this->faces[f_i].loops[p_i].vertices.size() > 0 && is_point_in_loop(f_i,p_i,v)==true)
        {
            if( this->is_closed_loop(f_i,p_i))
                n++;
            else
                n--;
        }
    }
    return n>0;
}

int polyfold::left_right_test(int f_i, int p_i, f32 vx, f32 vz, bool is_open, int n_tries) const
{
    f32 small_number = LEFT_RIGHT_TEST_SMALL_NUMBER;

    core::vector3df point;
    point.X = vx;
    point.Y = 0;
    point.Z = vz;

    std::vector<core::vector3df> verts;
    for(int v_i : this->faces[f_i].loops[p_i].vertices)
    {
        verts.push_back(this->vertices[v_i].V);
    }
    verts.push_back(verts[0]);

    core::matrix4 R = this->faces[f_i].get2Dmat();

    for(core::vector3df &v : verts)
    {
        R.rotateVect(v);
    }

    f32 closest_x=99000;
    f32 closest_z=99000;

    int status=-1; //-1 = undefined, 0 = left, 1 = right
    int try_status=-1;
    int x_status=-1;
    int z_status=-1;

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

        if(point.X > leftv.X  - small_number &&
           point.X < rightv.X + small_number &&
           fabs(rightv.X - leftv.X) > small_number)
        {
            f32 z_intersect;
            f32 m = (rightv.Z - leftv.Z)/ (rightv.X - leftv.X);
            z_intersect = m*(point.X-leftv.X)+leftv.Z;

           if(fabs(fabs(point.Z - z_intersect)-closest_z) < small_number)
           {
                if(verts[i].X < verts[i+1].X
                   && point.Z > z_intersect - small_number)
                {
                    try_status = 0; //Left
                }
                else if(verts[i].X < verts[i+1].X
                   && point.Z < z_intersect + small_number)
                {
                    try_status = 1; //Right
                }
                else if(verts[i].X > verts[i+1].X
                   && point.Z > z_intersect - small_number)
                {
                    try_status = 1; //Right
                }
                else if(verts[i].X > verts[i+1].X
                   && point.Z < z_intersect + small_number)
                {
                    try_status = 0; //Left
                }

                if(try_status != z_status)
                {
                    z_status = -1; //ambiguous
                }
           }
           else if(fabs(point.Z - z_intersect) < closest_z)
            {
                if(verts[i].X < verts[i+1].X
                   && point.Z > z_intersect - small_number)
                {
                    z_status = 0; //Left
                    closest_z = fabs(point.Z - z_intersect);
                }
                else if(verts[i].X < verts[i+1].X
                   && point.Z < z_intersect + small_number)
                {
                    z_status = 1; //Right
                    closest_z = fabs(point.Z - z_intersect);
                }
                else if(verts[i].X > verts[i+1].X
                   && point.Z > z_intersect - small_number)
                {
                    z_status = 1; //Right
                    closest_z = fabs(point.Z - z_intersect);
                }
                else if(verts[i].X > verts[i+1].X
                   && point.Z < z_intersect + small_number)
                {
                    z_status = 0; //Left
                    closest_z = fabs(point.Z - z_intersect);
                }
            }
        }
        if (point.Z > std::min(verts[i].Z,verts[i+1].Z) - small_number &&
           point.Z < std::max(verts[i].Z,verts[i+1].Z) + small_number &&
           fabs(rightv.Z - leftv.Z) > small_number)
        {
            f32 x_intersect;
            if(fabs(rightv.X - leftv.X) < small_number)
            {
                x_intersect = leftv.X;
            }
            else
            {
                f32 m = (rightv.Z - leftv.Z)/ (rightv.X - leftv.X);
                x_intersect = ((point.Z-leftv.Z))/m+leftv.X;
            }
            if(fabs(fabs(point.X - x_intersect)-closest_x) < small_number)
            {

                if(verts[i].Z < verts[i+1].Z
                   && point.X < x_intersect + small_number)
                {
                    try_status = 0; //Left
                }
                else if(verts[i].Z < verts[i+1].Z
                   && point.X > x_intersect - small_number)
                {
                    try_status = 1; //Right
                }
                else if(verts[i].Z > verts[i+1].Z
                   && point.X < x_intersect + small_number)
                {
                    try_status = 1; //Right
                }
                else if(verts[i].Z > verts[i+1].Z
                   && point.X > x_intersect - small_number)
                {
                    try_status = 0; //Left
                }

                if(try_status != x_status)
                {
                    x_status = -1; //ambiguous
                }
            }
            else if(fabs(point.X - x_intersect) < closest_x)
            {
                if(verts[i].Z < verts[i+1].Z
                   && point.X < x_intersect + small_number)
                {
                    x_status = 0; //Left
                    closest_x = fabs(point.X - x_intersect);
                }
                else if(verts[i].Z < verts[i+1].Z
                   && point.X > x_intersect - small_number)
                {
                    x_status = 1; //Right
                    closest_x = fabs(point.X - x_intersect);
                }
                else if(verts[i].Z > verts[i+1].Z
                   && point.X < x_intersect + small_number)
                {
                    x_status = 1; //Right
                    closest_x = fabs(point.X - x_intersect);
                }
                else if(verts[i].Z > verts[i+1].Z
                   && point.X > x_intersect - small_number)
                {
                    x_status = 0; //Left
                    closest_x = fabs(point.X - x_intersect);
                }
            }
        }
    }

    if(closest_x < LEFT_RIGHT_TEST_INCLUDE_NUMBER || closest_z < LEFT_RIGHT_TEST_INCLUDE_NUMBER)
    {
        x_status=1;
        z_status=1;
    }

    if(x_status != z_status && x_status !=-1 && z_status !=-1)
    {
        status = -1;
    }
    else if(x_status != -1) status = x_status;
    else if(z_status != -1) status = z_status;

    if(status == -1)
    {
        if(n_tries>0)
        {
            if(n_tries < 2)
            {
                std::cout<<"L/R TEST_1 attempt "<<vx<<","<<vz<<" ("<<n_tries<<")\n";
                std::cout<<"   x: "<<closest_x<<" / "<<x_status<<" "<<"z: "<<closest_z<<" / "<<z_status<<"\n";
                std::cout<<"bounds: "<<this->faces[f_i].loops[p_i].min_x<<","<<this->faces[f_i].loops[p_i].max_x<<
                                    "  "<<this->faces[f_i].loops[p_i].min_z<<","<<this->faces[f_i].loops[p_i].max_z<<"\n";
            }
            f32 newx = vx;
            f32 newz = vz;

            if(closest_x > closest_z)
            {
            newx = vx - 1;
            if(newx < this->faces[f_i].loops[p_i].min_x - small_number || newx > this->faces[f_i].loops[p_i].max_x + small_number)
                newx = vx + 1;
            }
            else
            {
            newz = vz - 1;
            if(newz < this->faces[f_i].loops[p_i].min_z - small_number || newz > this->faces[f_i].loops[p_i].max_z + small_number)
                newz = vz + 1;
            }

            return this->left_right_test(f_i,p_i,newx,newz, is_open, n_tries-1);
        }
        std::cout<<"**warning - undefined left right test (T2) "<<f_i<<" "<<p_i<<" **\n";

        for(core::vector3df v : verts)
        {
            std::cout<<v.X<<","<<v.Z<<" ";
        }
        std::cout<<"("<<vx<<","<<vz<<")";

        status = 1;
    }

   return status;
}


poly_vert polyfold::getVertex(int f_i, int e_i, int v_i) const
{
    if(v_i == 0)
        return this->vertices[this->edges[this->faces[f_i].edges[e_i]].v0];
    else if(v_i==1)
        return this->vertices[this->edges[this->faces[f_i].edges[e_i]].v1];
    else
        {
            return poly_vert(0,0,0);
        }
}

poly_vert polyfold::getVertex(int e_i, int v_i) const
{
    if(v_i == 0)
        return vertices[edges[e_i].v0];
    else if(v_i==1)
        return vertices[edges[e_i].v1];
    else
        {
            return poly_vert(0,0,0);
        }
}

std::vector<int> polyfold::get_edges_from_point(int f_i, int v) const
{
    std::vector<int> ret;

    for(int e_i : faces[f_i].edges)
    {
        if((edges[e_i].v0 == v || edges[e_i].v1 == v ) && edges[e_i].topo_group != -1)
        ret.push_back(e_i);
    }

    return ret;
}

int polyfold::get_opposite_end(int e_i, int v_i) const
{
    if (edges[e_i].v0 == v_i)
        return edges[e_i].v1;
    else return edges[e_i].v0;
}


void polyfold::get_all_edges_from_point(int e_0, int v, std::vector<int>& ret) const
{
    for(int e_i = 0; e_i<edges.size(); e_i++)
    {
        if((edges[e_i].v0 == v || edges[e_i].v1 == v ) && (e_i != e_0) && edges[e_i].topo_group != -1)
            ret.push_back(e_i);
    }
}


void polyfold::get_all_edges_from_point_accelerated(int e_0, int v, std::vector<int>& ret) const
{
    static std::vector<u16> hits;

    hits.clear();

    edges_BVH.intersect(vertices[v].V, hits);

    for (u16 e_i : hits)
    {
        if ((edges[e_i].v0 == v || edges[e_i].v1 == v) && (e_i != e_0) && edges[e_i].topo_group != -1)
            ret.push_back(e_i);
    }
}

/*
std::vector<int> polyfold::get_edges_from_2edges(int f_i, int e_0, int e_1)
{
    std::vector<int> ret;

    int v;

    if(this->edges[e_0].v0 == this->edges[e_1].v0 || this->edges[e_0].v0 == this->edges[e_1].v1)
        v = this->edges[e_0].v0;
    else
        v = this->edges[e_0].v1;

    for(int e_i : this->faces[f_i].edges)
    {
        if((this->edges[e_i].v0 == v || this->edges[e_i].v1 == v ) && (e_i != e_0) && (e_i != e_1) && this->edges[e_i].topo_group != -1)
            ret.push_back(e_i);
    }

    return ret;
}*/


std::vector<int> polyfold::get_connecting_edges(int edge_no) const
{
    std::vector<int> ret;

    int v0 = edges[edge_no].v0;
    int v1 = edges[edge_no].v1;

    for(int e_i = 0; e_i<edges.size(); e_i++)
    {
        if(e_i != edge_no && (edges[e_i].v0 == v0 || edges[e_i].v1 == v1 || edges[e_i].v0 == v1 || edges[e_i].v1 == v0))
        ret.push_back(e_i);
    }

    return ret;
}

bool polyfold::edge_exists(int v0,int v1) const
{
    for(int e_i=0; e_i<edges.size(); e_i++)
    {
        const poly_edge& edge = edges[e_i];
        if((edge.v0 == v0 && edge.v1 == v1) ||  (edge.v1 == v0 && edge.v0 == v1))
            return true;
    }
    return false;
}

int polyfold::get_edge_or_add(int v0,int v1,int group)
{
    for(int e_i=0; e_i<this->edges.size(); e_i++)
    {
        const poly_edge& edge = this->edges[e_i];
        if((edge.v0 == v0 && edge.v1 == v1) ||  (edge.v1 == v0 && edge.v0 == v1))
            return e_i;
    }
    this->edges.push_back(poly_edge(v0,v1));
    this->edges[this->edges.size()-1].topo_group = group;
    return this->edges.size()-1;
}


int polyfold::find_edge(int v0,int v1) const
{
    for(int e_i=0; e_i<this->edges.size(); e_i++)
    {
        const poly_edge& edge = this->edges[e_i];
        if((edge.v0 == v0 && edge.v1 == v1) ||  (edge.v1 == v0 && edge.v0 == v1))
            return e_i;
    }
    return -1;
}

int polyfold::find_edge(int f_i, int v0, int v1) const
{
    for (int e_i : faces[f_i].edges)
    {
        const poly_edge& edge = this->edges[e_i];
        if ((edge.v0 == v0 && edge.v1 == v1) || (edge.v1 == v0 && edge.v0 == v1))
            return e_i;
    }
    return -1;

}

int polyfold::find_edge(core::vector3df v0, core::vector3df v1) const
{
    for(int e_i=0; e_i<this->edges.size(); e_i++)
    {
        if((is_same_point(this->getVertex(e_i,0).V, v0) &&  is_same_point(this->getVertex(e_i,1).V, v1))
           || (is_same_point(this->getVertex(e_i,1).V, v0) &&  is_same_point(this->getVertex(e_i,0).V, v1)))
            return e_i;
    }
    return -1;
}


bool polyfold::point_is_coplanar(int face, core::vector3df v )
{
    core::plane3df plane = core::plane3df(this->faces[face].m_center,this->faces[face].m_normal);
    if(fabs(plane.getDistanceTo(v))<POINT_IS_COPLANAR_DIST)
        return true;
    else return false;
}
bool polyfold::point_is_on_edge(int edge, core::vector3df v )
{
    core::line3df line = core::line3df(this->getVertex(edge,0).V,this->getVertex(edge,1).V);
    core::vector3df dist = line.getClosestPoint(v) - v;
    if(dist.getLength()<POINT_IS_COLINEAR_DIST)
        return true;
    else
        return false;
}


void polyfold::build_edges_BVH()
{
    edges_BVH.invalidate();
    bool (*test_func)(const poly_edge*);

    test_func = [](const poly_edge* edge) -> bool {
        return edge->topo_group != -1;
        };

    edges_BVH.build(vertices.data(), edges.data(), edges.size(),test_func);
}

void polyfold::build_faces_BVH()
{
    faces_BVH.invalidate();
    faces_BVH.build(vertices.data(), faces.data(), faces.size());
}

void polyfold::build_vertices_BVH()
{
    vertices_BVH.invalidate();
    vertices_BVH.build(vertices.data(), vertices.size());
}



surface_group polyfold::getFaceSurfaceGroup(int f_i)
{
    int sfg = this->faces[f_i].surface_group;
    return this->surface_groups[sfg];
}

bool polyfold::getSurfaceVectors(int f_i,core::vector3df &a, core::vector3df &b)
{
    surface_group sfg=surface_groups[faces[f_i].surface_group];

    core::vector3df N = faces[f_i].m_normal;
    core::vector3df iZ =  is_parallel_normal(sfg.vec,N) ? sfg.vec1.crossProduct(N) : sfg.vec.crossProduct(N);
    iZ.normalize();
    core::vector3df iX = iZ.crossProduct(N);

    a = iZ;
    b = iX;
    return true;
}

void polyfold::addDrawLinesEdges(LineHolder& graph) const
{
    for (int e_i=0;e_i<edges.size();e_i++)
        graph.lines.push_back(core::line3df(getVertex(e_i, 0).V, getVertex(e_i, 1).V));
}

void polyfold::addDrawLinesEdgesByIndex(std::vector<u16> edges_i, LineHolder& graph) const
{
    for (u16 e_i : edges_i)
        graph.lines.push_back(core::line3df(getVertex(e_i, 0).V, getVertex(e_i, 1).V));
}

void polyfold::addDrawLinesFacesByIndex(std::vector<u16> faces_i, LineHolder& graph) const
{
    for (u16 f_i : faces_i)
    {
        for (u16 e_i : faces[f_i].edges)
            graph.lines.push_back(core::line3df(getVertex(e_i, 0).V, getVertex(e_i, 1).V));
    }
}

REFLECT_STRUCT_BEGIN(point_texcoord)
    REFLECT_STRUCT_MEMBER(vertex)
    REFLECT_STRUCT_MEMBER(texcoord)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(poly_vert)
    REFLECT_STRUCT_MEMBER(V)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(poly_loop)
    REFLECT_STRUCT_MEMBER(vertices)
    REFLECT_STRUCT_MEMBER(type)
    REFLECT_STRUCT_MEMBER(topo_group)
    REFLECT_STRUCT_MEMBER(depth)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(surface_group)
    REFLECT_STRUCT_MEMBER(type)
    REFLECT_STRUCT_MEMBER(point)
    REFLECT_STRUCT_MEMBER(vec)
    REFLECT_STRUCT_MEMBER(vec1)
    REFLECT_STRUCT_MEMBER(texcoords)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(poly_face)
    //REFLECT_STRUCT_MEMBER(edges)
    //REFLECT_STRUCT_MEMBER(vertices)
    REFLECT_STRUCT_MEMBER(loops)
    REFLECT_STRUCT_MEMBER(m_normal)
    //REFLECT_STRUCT_MEMBER(m_center)
    REFLECT_STRUCT_MEMBER(texture_index)
    REFLECT_STRUCT_MEMBER(surface_group)
    REFLECT_STRUCT_MEMBER(material_group)
    REFLECT_STRUCT_MEMBER(uv_origin)
    REFLECT_STRUCT_MEMBER(bFlippedNormal)
    REFLECT_STRUCT_MEMBER(uv_mat)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(polyfold)
    REFLECT_STRUCT_MEMBER(vertices)
    REFLECT_STRUCT_MEMBER(faces)
    REFLECT_STRUCT_MEMBER(surface_groups)
    REFLECT_STRUCT_MEMBER(topology)
REFLECT_STRUCT_END()
