
#include <irrlicht.h>
#include "tolerances.h"
#include "polyfold_soa.h"

using namespace irr;
using namespace core;

#define small_number LEFT_RIGHT_TEST_SMALL_NUMBER

int polyfold_soa::left_right_test(u32 f_i, u32 p_i, vector2df r, int n_tries) const
{
    core::vector3df point;
    point.X = r.X;
    point.Y = 0;
    point.Z = r.Y;

    std::vector<core::vector3df> verts;

    u32 p_idx = loop.index.offset[f_i];
    u32 n_verts = loop.vertices.index.idx1.len[p_idx + p_i];
    u32 f0 = loop.vertices.index.idx0.offset[f_i];
    u32 p0 = f0 + loop.vertices.index.idx1.offset[p_idx + p_i];
   
    verts.resize(n_verts);

    for (int i = 0; i < verts.size(); i++)
    {
        u32 v_i = loop.vertices[p0 + i];
        verts[i] = vertices[v_i].V;
    }

    for (core::vector3df& v : verts)
    {
        face.mat2D[f_i].rotateVect(v);
    }

    f32 closest_x = 99000;
    f32 closest_z = 99000;

    int status = -1; //-1 = undefined, 0 = left, 1 = right
    int try_status = -1;
    int x_status = -1;
    int z_status = -1;

    // A scan line that misses the loop's extent can only graze the extreme vertex within
    // tolerance. That gives a phantom crossing on one of the vertex's two edges (e.g. a
    // nearly-horizontal edge is skipped and only its neighbour counts), which contradicts
    // the other scan. The callers' bbox tests admit points up to LOOP_BBOX_SMALL_NUMBER
    // outside the loop, so such a scan must be ignored and the other scan decides.
    // (Same fix as polyfold::left_right_test.)
    f32 loop_min_x = verts[0].X, loop_max_x = verts[0].X;
    f32 loop_min_z = verts[0].Z, loop_max_z = verts[0].Z;
    for (const core::vector3df& v : verts)
    {
        loop_min_x = std::min(loop_min_x, v.X);
        loop_max_x = std::max(loop_max_x, v.X);
        loop_min_z = std::min(loop_min_z, v.Z);
        loop_max_z = std::max(loop_max_z, v.Z);
    }
    const bool x_scan_valid = point.Z >= loop_min_z && point.Z <= loop_max_z; //scan along X at point.Z
    const bool z_scan_valid = point.X >= loop_min_x && point.X <= loop_max_x; //scan along Z at point.X

    if (!x_scan_valid && !z_scan_valid)
    {
        // diagonally beyond a corner of the bbox: inside only if within the include distance of a vertex
        for (const core::vector3df& v : verts)
        {
            if (fabs(v.X - point.X) < LEFT_RIGHT_TEST_INCLUDE_NUMBER && fabs(v.Z - point.Z) < LEFT_RIGHT_TEST_INCLUDE_NUMBER)
                return 1;
        }
        return 0;
    }

    for (int i = 0; i < verts.size() - 1; i++)
    {
        core::vector3df leftv, rightv;
        if (verts[i].X < verts[i + 1].X)
        {
            leftv = verts[i];
            rightv = verts[i + 1];
        }
        else
        {
            leftv = verts[i + 1];
            rightv = verts[i];
        }

        if (z_scan_valid &&
            point.X > leftv.X - small_number &&
            point.X < rightv.X + small_number &&
            fabs(rightv.X - leftv.X) > small_number)
        {
            f32 z_intersect;
            f32 m = (rightv.Z - leftv.Z) / (rightv.X - leftv.X);
            z_intersect = m * (point.X - leftv.X) + leftv.Z;

            if (fabs(fabs(point.Z - z_intersect) - closest_z) < small_number)
            {
                if (verts[i].X < verts[i + 1].X
                    && point.Z > z_intersect - small_number)
                {
                    try_status = 0; //Left
                }
                else if (verts[i].X < verts[i + 1].X
                    && point.Z < z_intersect + small_number)
                {
                    try_status = 1; //Right
                }
                else if (verts[i].X > verts[i + 1].X
                    && point.Z > z_intersect - small_number)
                {
                    try_status = 1; //Right
                }
                else if (verts[i].X > verts[i + 1].X
                    && point.Z < z_intersect + small_number)
                {
                    try_status = 0; //Left
                }

                if (try_status != z_status)
                {
                    z_status = -1; //ambiguous
                }
            }
            else if (fabs(point.Z - z_intersect) < closest_z)
            {
                if (verts[i].X < verts[i + 1].X
                    && point.Z > z_intersect - small_number)
                {
                    z_status = 0; //Left
                    closest_z = fabs(point.Z - z_intersect);
                }
                else if (verts[i].X < verts[i + 1].X
                    && point.Z < z_intersect + small_number)
                {
                    z_status = 1; //Right
                    closest_z = fabs(point.Z - z_intersect);
                }
                else if (verts[i].X > verts[i + 1].X
                    && point.Z > z_intersect - small_number)
                {
                    z_status = 1; //Right
                    closest_z = fabs(point.Z - z_intersect);
                }
                else if (verts[i].X > verts[i + 1].X
                    && point.Z < z_intersect + small_number)
                {
                    z_status = 0; //Left
                    closest_z = fabs(point.Z - z_intersect);
                }
            }
        }
        if (x_scan_valid &&
            point.Z > std::min(verts[i].Z, verts[i + 1].Z) - small_number &&
            point.Z < std::max(verts[i].Z, verts[i + 1].Z) + small_number &&
            fabs(rightv.Z - leftv.Z) > small_number)
        {
            f32 x_intersect;
            if (fabs(rightv.X - leftv.X) < small_number)
            {
                x_intersect = leftv.X;
            }
            else
            {
                f32 m = (rightv.Z - leftv.Z) / (rightv.X - leftv.X);
                x_intersect = ((point.Z - leftv.Z)) / m + leftv.X;
            }
            if (fabs(fabs(point.X - x_intersect) - closest_x) < small_number)
            {

                if (verts[i].Z < verts[i + 1].Z
                    && point.X < x_intersect + small_number)
                {
                    try_status = 0; //Left
                }
                else if (verts[i].Z < verts[i + 1].Z
                    && point.X > x_intersect - small_number)
                {
                    try_status = 1; //Right
                }
                else if (verts[i].Z > verts[i + 1].Z
                    && point.X < x_intersect + small_number)
                {
                    try_status = 1; //Right
                }
                else if (verts[i].Z > verts[i + 1].Z
                    && point.X > x_intersect - small_number)
                {
                    try_status = 0; //Left
                }

                if (try_status != x_status)
                {
                    x_status = -1; //ambiguous
                }
            }
            else if (fabs(point.X - x_intersect) < closest_x)
            {
                if (verts[i].Z < verts[i + 1].Z
                    && point.X < x_intersect + small_number)
                {
                    x_status = 0; //Left
                    closest_x = fabs(point.X - x_intersect);
                }
                else if (verts[i].Z < verts[i + 1].Z
                    && point.X > x_intersect - small_number)
                {
                    x_status = 1; //Right
                    closest_x = fabs(point.X - x_intersect);
                }
                else if (verts[i].Z > verts[i + 1].Z
                    && point.X < x_intersect + small_number)
                {
                    x_status = 1; //Right
                    closest_x = fabs(point.X - x_intersect);
                }
                else if (verts[i].Z > verts[i + 1].Z
                    && point.X > x_intersect - small_number)
                {
                    x_status = 0; //Left
                    closest_x = fabs(point.X - x_intersect);
                }
            }
        }
    }

    if (closest_x < LEFT_RIGHT_TEST_INCLUDE_NUMBER || closest_z < LEFT_RIGHT_TEST_INCLUDE_NUMBER)
    {
        x_status = 1;
        z_status = 1;
    }

    if (x_status != z_status && x_status != -1 && z_status != -1)
    {
        status = -1;
    }
    else if (x_status != -1) status = x_status;
    else if (z_status != -1) status = z_status;

    if (status == -1)
    {
        if (n_tries > 0)
        {
            f32 newx = r.X;
            f32 newz = r.Y;

            if (closest_x > closest_z)
            {
                newx = r.X - 1;
                if (newx < loop.loop_info[p_idx + p_i].aabb.min_x - small_number || newx > loop.loop_info[p_idx + p_i].aabb.max_x + small_number)
                    newx = r.X + 1;
            }
            else
            {
                newz = r.Y - 1;
                if (newz < loop.loop_info[p_idx + p_i].aabb.min_z - small_number || newz > loop.loop_info[p_idx + p_i].aabb.max_z + small_number)
                    newz = r.Y + 1;
            }

            return this->left_right_test(f_i, p_i, vector2df(newx, newz), n_tries - 1);
        }
        std::cout << "**warning - undefined left right test ** \n";

        status = 1;
    }

    return status;
}