#ifndef _LIGHTMAPS_H_
#define _LIGHTMAPS_H_

#include <irrlicht.h>
#include "csg_classes.h"
#include "BufferManager.h"

class geometry_scene;

void guess_lightmaps_dimension(geometry_scene* geo_scene, int f_j);
void lightmaps_divideMaterialGroups2(geometry_scene* geo_scene, std::vector<TextureMaterial>& material_groups);




#endif