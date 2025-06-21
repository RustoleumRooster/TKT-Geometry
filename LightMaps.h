#ifndef _LIGHTMAPS_H_
#define _LIGHTMAPS_H_

#include <irrlicht.h>
#include <vector>
#include "BufferManager.h"

class GeometryStack;
class geometry_scene;

void lightmaps_divideMaterialGroups(GeometryStack* geo_scene, std::vector<TextureMaterial>& material_groups);

class Lightmap_Manager
{
public:
	Lightmap_Manager();

	void loadLightmapTextures(geometry_scene* geo_scene);

	std::vector<irr::video::ITexture*> lightmap_textures;
	MyEventReceiver* event_receiver = NULL;
};

class Lightmaps_Tool
{
public:
	static Lightmap_Manager* lightmap_manager;

	static void set_manager(Lightmap_Manager* lmm)
	{
		lightmap_manager = lmm;
	}

	static Lightmap_Manager* get_manager()
	{
		return lightmap_manager;
	}

};
#endif