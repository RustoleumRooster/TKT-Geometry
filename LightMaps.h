#ifndef _LIGHTMAPS_H_
#define _LIGHTMAPS_H_

#include <irrlicht.h>
#include <vector>
#include "BufferManager.h"
#include <iterator>

class GeometryStack;
class geometry_scene;

void layout_lightmaps(GeometryStack*, std::vector<TextureMaterial>& material_groups);
void split_material_groups(GeometryStack*, const std::vector<TextureMaterial>& material_groups);
void calc_lightmap_uvs(GeometryStack*, Lightmap_Block b);
void initialize_lightmap_block(GeometryStack*, int element_id, int surface_no, std::back_insert_iterator<std::vector<Lightmap_Block>> ret, int reduce);

class Lightmap_Configuration;

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

class Lightmap_Configuration
{
	struct min_lm_block
	{
		rect<u16> coords;
		bool bFlipped = false;
	};

public:

	//Lightmap_Configuration(GeometryStack* geo) : geo_node(geo)
	//{}
	void initialize(GeometryStack* geo_node_) { geo_node = geo_node_; }
	void layout_lightmaps();
	void split_material_groups(const std::vector<TextureMaterial>& material_groups);
	void apply_transforms_to_mesh(irr::scene::SMesh*);

	std::vector<TextureMaterial> materials;
	std::vector<min_lm_block> lightmap_blocks;
	GeometryStack* geo_node = NULL;
};




#endif