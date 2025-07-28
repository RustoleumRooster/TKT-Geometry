#ifndef _LIGHTMAPS_H_
#define _LIGHTMAPS_H_

#include <irrlicht.h>
#include <vector>
#include "BufferManager.h"
#include <iterator>

class GeometryStack;
class geometry_scene;

void layout_lightmaps(GeometryStack* geo_scene, std::vector<TextureMaterial>& material_groups);
void split_material_groups(GeometryStack* geo_scene, std::vector<TextureMaterial>& material_groups);
void calc_lightmap_uvs(GeometryStack* geo_node, Lightmap_Block b);
void initialize_lightmap_block(GeometryStack* geo_node, int element_id, int surface_no, std::back_insert_iterator<std::vector<Lightmap_Block>> ret, int reduce);

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