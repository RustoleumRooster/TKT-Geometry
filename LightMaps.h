#ifndef _LIGHTMAPS_H_
#define _LIGHTMAPS_H_

#include <irrlicht.h>
#include <vector>
#include "BufferManager.h"
#include <iterator>

class GeometryStack;
class geometry_scene;

//void calc_lightmap_uvs(GeometryStack*, Lightmap_Block b);
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

struct  tex_block
{
	dimension2du dimension;
	int element_id = -1;
	int surface_no = -1;
	bool bFlipped = false;
	rect<u16> coords;
	int size() { return dimension.Width * dimension.Height; }
};

struct blocklist
{
	vector<tex_block> blocks;

	int size = 0;
	vector<int> material_groups;

	void calc_size();
	void sort();
	void operator+=(blocklist& other);
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
	void split_material_groups();
	void apply_transforms_to_mesh(irr::scene::SMesh*);
	void apply_lightmap_uvs_to_mesh();
	void set_lightmap_uvs_to_mesh(dimension2du lm_dimension, tex_block& block);
	void set_reduction(int);
	void initialize_soa_arrays(scene::SMesh* mesh);

	void calc_lightmap_uvs(const std::vector<TextureMaterial>& material_groups);
	void calc_lightmap_uvs(GeometryStack* geo_node, Lightmap_Block b);

	template<class map_type>
	void map_uvs(MeshNode_Interface_Edit* mesh_node, int surface_offset, const std::vector<int>& surface, map_type& mapper, int uv_type);

	void copy_raw_lightmap_uvs_to_mesh(MeshNode_Interface_Edit* mesh_node, const std::vector<int>& surface);

	const std::vector<TextureMaterial>& get_materials(){ return materials; }

	//================================
	//structs for holding lightmap uvs
	soa_struct_2<vector3df, vector2df> lm_raw_uvs;
	soa_struct<u16> indices;

	std::vector<TextureMaterial> materials;
	std::vector<min_lm_block> lightmap_blocks;
	vector<blocklist> bl_combined;

	GeometryStack* geo_node = NULL;

	int reduce_power = 0;
};

template<class map_type>
void Lightmap_Configuration::map_uvs(MeshNode_Interface_Edit* mesh_node, int surface_offset, const std::vector<int>& surface, map_type& mapper, int uv_type)
{
	for (int b_i : surface)
	{
		int f_j = mesh_node->get_buffer_index_by_face(surface_offset + b_i);

		int offset = this->indices.offset[f_j];
		int len = this->indices.len[f_j];

		for (int i = offset; i < offset + len; i += 3)
		{

			mapper.calc(this->lm_raw_uvs.data0.data(),		//vertices
				this->lm_raw_uvs.data1.data(),		//uvs
				&this->indices.data[i]);			//indices

			//std::cout << " " << vtx[0]->TCoords2.X << "," << vtx[0]->TCoords2.Y << "  ";
			//std::cout << " " << vtx[1]->TCoords2.X << "," << vtx[1]->TCoords2.Y << "  ";
			//std::cout << " " << vtx[2]->TCoords2.X << "," << vtx[2]->TCoords2.Y << "\n";
		}
	}
}




#endif