#ifndef _LIGHTMAPS_H_
#define _LIGHTMAPS_H_

#include <irrlicht.h>
#include "csg_classes.h"
#include "BufferManager.h"
#include <vector>

class geometry_scene;

void lightmaps_divideMaterialGroups(geometry_scene* geo_scene, std::vector<TextureMaterial>& material_groups);



class Lightmap_Manager
{
public:
	void loadLightmapTextures(geometry_scene* geo_scene, const std::vector<TextureMaterial>& material_groups);

	std::vector<irr::video::ITexture*> lightmap_textures;
};
#endif