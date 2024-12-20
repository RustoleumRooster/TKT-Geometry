#ifndef _BUILD_MESHES_H_
#define _BUILD_MESHES_H_

#include <irrlicht.h>
#include <vector>
#include "csg_classes.h"

using namespace irr;

class geometry_scene;
class scene::IMeshBuffer;

void make_meshbuffer_from_triangles(const std::vector<triangle_holder>&, scene::IMeshBuffer*);
void make_meshbuffer_from_triangles(const triangle_holder&, scene::IMeshBuffer*);

void calculate_meshbuffer_uvs_cube(GeometryStack* g_scene, int b_i, int f_i, scene::IMeshBuffer* buffer);
void calculate_meshbuffer_uvs_cylinder(GeometryStack* g_scene, int b_i, int f_i, scene::IMeshBuffer* buffer);
void calculate_meshbuffer_uvs_dome(GeometryStack* g_scene, int b_i, int f_i, scene::IMeshBuffer* buffer);
void calculate_meshbuffer_uvs_sphere(GeometryStack* g_scene, int b_i, int f_i, scene::IMeshBuffer* buffer);
void calculate_meshbuffer_uvs_custom(GeometryStack* g_scene, int b_i, int f_i, scene::IMeshBuffer* buffer);

void calculate_meshbuffer_uvs_cube(GeometryStack* g_scene, int b_i, int f_i, scene::IMeshBuffer* buffer, int, int);
void calculate_meshbuffer_uvs_cylinder(GeometryStack* g_scene, int b_i, int f_i, scene::IMeshBuffer* buffer, int, int);
void calculate_meshbuffer_uvs_dome(GeometryStack* g_scene, int b_i, int f_i, scene::IMeshBuffer* buffer, int, int);
void calculate_meshbuffer_uvs_sphere(GeometryStack* g_scene, int b_i, int f_i, scene::IMeshBuffer* buffer, int, int);
void calculate_meshbuffer_uvs_custom(GeometryStack* g_scene, int b_i, int f_i, scene::IMeshBuffer* buffer, int, int);


#endif