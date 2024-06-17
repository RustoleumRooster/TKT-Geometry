#ifndef _TOLERANCES_H_
#define _TOLERANCES_H_

#include <irrlicht.h>
using namespace irr;

#define LOOP_BBOX_SMALL_NUMBER 0.01
#define LEFT_RIGHT_TEST_SMALL_NUMBER 0.005
#define LEFT_RIGHT_TEST_INCLUDE_NUMBER 0.01
#define POINT_IS_COPLANAR_DIST 0.01
#define POINT_IS_COLINEAR_DIST 0.01
#define LINE_IS_COPLANAR_DOT 0.001

#define BOX_INTERSECT_TOLERANCE 0.1

#define USE_DOUBLE_FOR_PARALLELISM
#define DOUBLE_PARRALELISM_EPSILON 0.00001
#define FLOAT_PARRALELISM_EPSILON 0.0001
//#define PLANE_IS_COPLANAR_DOT 0.9999

bool is_same_point(const core::vector3df& v0, const core::vector3df& v1);
bool is_same_point(const core::vector2df& v0, const core::vector2df& v1);
bool is_same_point(f32 f1, f32 f2);
bool is_colinear_point(const core::line3df&, const core::vector3df&);
bool is_coplanar_point(const core::plane3df&, const core::vector3df&);

//assumes normalized
bool is_parallel_normal(const core::vector3df&, const core::vector3df&);

#endif
