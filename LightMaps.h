#ifndef _LIGHTMAPS_H_
#define _LIGHTMAPS_H_

#include <irrlicht.h>
#include "csg_classes.h"
#include "BufferManager.h"
#include <vector>

class GeometryStack;

void lightmaps_divideMaterialGroups(GeometryStack* geo_scene, std::vector<TextureMaterial>& material_groups);



class Lightmap_Manager
{
public:
	Lightmap_Manager();

	void loadLightmapTextures(GeometryStack* geo_scene, const std::vector<TextureMaterial>& material_groups);

	std::vector<irr::video::ITexture*> lightmap_textures;
	MyEventReceiver* event_receiver = NULL;
};
#endif