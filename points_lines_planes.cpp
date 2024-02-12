
#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"
#include "tolerances.h"
//#include "BVH.h"

using namespace irr;


bool is_same_point(const core::vector3df& v0, const core::vector3df& v1)
{
    if (fabs(v0.X - v1.X) < 0.25 &&
        fabs(v0.Y - v1.Y) < 0.25 &&
        fabs(v0.Z - v1.Z) < 0.25)
    {
        return true;
    }
    /*
    if (fabs(v0.X - v1.X) < 1.0 &&
        fabs(v0.Y - v1.Y) < 1.0 &&
        fabs(v0.Z - v1.Z) < 1.0)
    {
        // return true;
        std::cout << "*warning* is_same_point(" << v0.X << "," << v1.X << " " << v0.Y << "," << v1.Y << " " << v0.Z << "," << v1.Z << ")\n";
    }*/
    return false;
}

bool is_same_point(const core::vector2df& v0, const core::vector2df& v1)
{
    if (fabs(v0.X - v1.X) < 0.02 &&
        fabs(v0.Y - v1.Y) < 0.02)
    {
        return true;
    }
    return false;
}

bool is_same_point(f32 f1, f32 f2)
{
    if (fabs(f2 - f1) < 0.02)
    {
        return true;
    }
    return false;
}

bool is_colinear_point(const core::line3df& line, const core::vector3df& v)
{
    core::vector3df dist = line.getClosestPoint(v) - v;
    if (dist.getLength() < POINT_IS_COLINEAR_DIST)
        return true;
    if (dist.getLength() < 1.0)
    {
        // std::cout<<"*warning* is_colinear_point "<<dist.getLength()<<"\n";
    }
    return false;
}

bool is_coplanar_point(const core::plane3df& plane, const core::vector3df& v)
{
    f32 dist = fabs(plane.getDistanceTo(v));
    if (dist < POINT_IS_COPLANAR_DIST)
        return true;
    if (dist < 0.1)
    {
        //  std::cout<<"*warning* is_coplanar_point "<<dist<<"\n";
    }
    return false;
}
#ifdef USE_DOUBLE_FOR_PARALLELISM
bool is_parallel_normal(const core::vector3df& v0, const core::vector3df& v1)
{
    double x1 = v0.X;
    double x2 = v1.X;
    double y1 = v0.Y;
    double y2 = v1.Y;
    double z1 = v0.Z;
    double z2 = v1.Z;

    double dp = x1 * x2 + y1 * y2 + z1 * z2;

    dp = fabs(dp);

    return (double)1.0 - dp < DOUBLE_PARRALELISM_EPSILON;
}

#else
bool is_parallel_normal(const core::vector3df& v0, const core::vector3df& v1)
{

    f32 dp = fabs(v0.dotProduct(v1));
    if (dp > PLANE_IS_COPLANAR_DOT)
    {
        if (dp < 0.9999)
        {
            //std::cout<<"**warning coplanar : "<<dp<<"(returned true) ";
            //std::cout<<v0.X<<","<<v0.Y<<","<<v0.Z<<" vs "<<v1.X<<","<<v1.Y<<","<<v1.Z<<"\n";
        }
        return true;
    }
    return false;
}
#endif

bool BoxIntersectsWithBox(const core::aabbox3d<f32>& A, const core::aabbox3d<f32>& B)
{
    return (A.MinEdge.X <= B.MaxEdge.X + BOX_INTERSECT_TOLERANCE &&
        A.MinEdge.Y <= B.MaxEdge.Y + BOX_INTERSECT_TOLERANCE &&
        A.MinEdge.Z <= B.MaxEdge.Z + BOX_INTERSECT_TOLERANCE &&
        A.MaxEdge.X >= B.MinEdge.X - BOX_INTERSECT_TOLERANCE &&
        A.MaxEdge.Y >= B.MinEdge.Y - BOX_INTERSECT_TOLERANCE &&
        A.MaxEdge.Z >= B.MinEdge.Z - BOX_INTERSECT_TOLERANCE);
}


bool line_intersects_plane(const core::plane3df& plane, const core::vector3df& v0, const core::vector3df& v1, core::vector3df& ipoint)
{
    core::vector3df linevec = v1 - v0;
    linevec.normalize();

    if (fabs(plane.Normal.dotProduct(linevec)) < LINE_IS_COPLANAR_DOT)
        return false;
    else if (plane.getIntersectionWithLimitedLine(v0, v1, ipoint))
        return true;
    else if (fabs(plane.getDistanceTo(v0)) < POINT_IS_COPLANAR_DIST && fabs(plane.getDistanceTo(v1)) > POINT_IS_COPLANAR_DIST)
    {
        ipoint = v0;
        return true;
    }
    else if (fabs(plane.getDistanceTo(v1)) < POINT_IS_COPLANAR_DIST && fabs(plane.getDistanceTo(v0)) > POINT_IS_COPLANAR_DIST)
    {
        ipoint = v1;
        return true;
    }
    return false;
}