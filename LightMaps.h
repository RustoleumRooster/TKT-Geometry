#ifndef _LIGHTMAPS_H_
#define _LIGHTMAPS_H_

#include <irrlicht.h>
#include "csg_classes.h"
#include "BufferManager.h"

class geometry_scene;

void lightmaps_divideMaterialGroups(geometry_scene* geo_scene, std::vector<TextureMaterial>& material_groups);




#endif