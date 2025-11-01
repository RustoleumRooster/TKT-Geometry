#include <irrlicht.h>
#include "LightMaps.h"
#include "csg_classes.h"
#include "utils.h"
#include "geometry_scene.h"
#include "tolerances.h"
#include <algorithm>
#include <math.h>
#include "BufferManager.h"
#include "uv_mapping.h"
#include <sstream>
#include "edit_env.h"
#include "CMeshSceneNode.h"
#include "file_open.h"
#include "vkModules.h"

using namespace irr;
using namespace core;
using namespace std;

Lightmap_Manager* Lightmaps_Tool::lightmap_manager = NULL;

extern IrrlichtDevice* device;

#define PRINTV(x) x.X <<","<<x.Y<<","<<x.Z<<" "

Lightmap_Manager::Lightmap_Manager()
{
	MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
	event_receiver = receiver;
}

void Lightmap_Manager::setLightmapTextures(geometry_scene* geo_scene, const std::vector<video::ITexture*>& new_textures)
{
	GeometryStack* geo_node = geo_scene->geoNode();
	const std::vector<TextureMaterial>& material_groups = geo_node->final_meshnode_interface.get_materials_used();

	SEvent event;
	event.EventType = EET_USER_EVENT;
	event.UserEvent.UserData1 = USER_EVENT_CLEAR_LIGHTMAP_TEXTURES;
	event_receiver->OnEvent(event);

	int N_lightmaps = 0;
	for (int i = 0; i < material_groups.size(); i++)
	{
		N_lightmaps = std::max(N_lightmaps, material_groups[i].lightmap_no);
	}
	N_lightmaps += 1;

	for (int i = 0; i < material_groups.size(); i++)
	{
		if (material_groups[i].has_lightmap == false || material_groups[i].lightmap_no >= new_textures.size())
		{
			continue;
		}

		video::ITexture* tex = new_textures[material_groups[i].lightmap_no];

		for (int f_i : material_groups[i].faces)
		{
			MeshBuffer_Chunk chunk;

			chunk = geo_node->final_meshnode_interface.get_mesh_buffer(f_i);
			chunk.buffer->getMaterial().setTexture(1, tex);

			int f_j = geo_node->final_meshnode_interface.edit_mb_buffer[f_i];
			chunk = geo_node->edit_meshnode_interface.get_mesh_buffer(f_j);
			chunk.buffer->getMaterial().setTexture(1, tex);
		}
	}

	geo_node->getMeshNode()->copyMaterials();

	event.EventType = EET_USER_EVENT;
	event.UserEvent.UserData1 = USER_EVENT_MATERIAL_GROUP_SELECTION_CHANGED;
	event_receiver->OnEvent(event);
}

void Lightmap_Manager::loadLightmapTextures(geometry_scene* geo_scene)
{
	
	return;
	GeometryStack* geo_node = geo_scene->geoNode();
	const std::vector<TextureMaterial>& material_groups = geo_node->final_meshnode_interface.get_materials_used();

	SEvent event;
	event.EventType = EET_USER_EVENT;
	event.UserEvent.UserData1 = USER_EVENT_CLEAR_LIGHTMAP_TEXTURES;
	event_receiver->OnEvent(event);

	//for (video::ITexture* tex : lightmap_textures)
	{
	//	device->getVideoDriver()->removeTexture(tex);
	}

	//lightmap_textures.clear();

	int N_lightmaps = 0;
	for (int i = 0; i < material_groups.size(); i++)
	{
		N_lightmaps = std::max(N_lightmaps, material_groups[i].lightmap_no);
	}
	N_lightmaps += 1;

	//lightmap_textures.resize(N_lightmaps);

	for (int i = 0; i < material_groups.size(); i++)
	{
		if (material_groups[i].has_lightmap == false)
		{
			continue;
		}

		std::stringstream ss;

		io::path p = File_Open_Tool::get_base()->GetCurrentProjectPath();

		ss << p.c_str();
		ss << geo_scene->get_lightmap_file_string() << "_" << material_groups[i].lightmap_no << ".bmp";

		video::ITexture* tex = device->getVideoDriver()->getTexture(ss.str().c_str());

		//lightmap_textures[material_groups[i].lightmap_no] = tex;

		for (int f_i : material_groups[i].faces)
		{
			MeshBuffer_Chunk chunk;

			chunk = geo_node->final_meshnode_interface.get_mesh_buffer(f_i);
			chunk.buffer->getMaterial().setTexture(1, tex);

			int f_j = geo_node->final_meshnode_interface.edit_mb_buffer[f_i];
			chunk = geo_node->edit_meshnode_interface.get_mesh_buffer(f_j);
			chunk.buffer->getMaterial().setTexture(1, tex);
		}
	}

	geo_node->getMeshNode()->copyMaterials();

	event.EventType = EET_USER_EVENT;
	event.UserEvent.UserData1 = USER_EVENT_MATERIAL_GROUP_SELECTION_CHANGED;
	event_receiver->OnEvent(event);
	
}


void Lightmap_Configuration::run_sunlight(geometry_scene* geo_scene)
{
	for (video::ITexture* tex : my_lightmaps)
	{
		device->getVideoDriver()->removeTexture(tex);
	}

	my_lightmaps.clear();

	Lightmap_Routine3(geo_scene, this, my_lightmaps,&geo_scene->geoNode()->get_lightmap_config(1));
	//Lightmap_Routine2(geo_scene, this, my_lightmaps,&geo_scene->geoNode()->get_lightmap_config(1));
	//Lightmap_Routine2(geo_scene, this, my_lightmaps, this);

}

void Lightmap_Configuration::apply_lightmaps(geometry_scene* geo_scene)
{
	Lightmaps_Tool::get_manager()->setLightmapTextures(geo_scene, my_lightmaps);
}

f32 reduce_dimension_base2(f32 dim, int n = 1)
{
	double log_width = log2(dim);
	u16 lwi = static_cast<u16>(floor(log_width));
	lwi -= n;
	lwi = lwi > 3 ? lwi : 3;
	lwi = lwi > 8 ? 8 : lwi;

	return exp2(lwi);
}

u16 get_dimension_base2(f32 dim)
{
	double log_width = log2(dim);
	return static_cast<u16>(floor(log_width));
}

bool is_reducible (f32 dim, int n = 1)
{
	double log_width = log2(dim);
	u16 lwi = static_cast<u16>(floor(log_width));
	lwi -= n;
	return lwi >= 3;
}

void blocklist::calc_size()
{
	size = 0;

	for (tex_block& b : blocks)
	{
		size += b.dimension.Width * b.dimension.Height;
	}
}

void blocklist::sort()
{
	std::sort(blocks.begin(), blocks.end(),
		[&](tex_block& a, tex_block& b)
		{
			if (a.dimension.Width == b.dimension.Width)
			{
				return a.dimension.Height > b.dimension.Height;
			}
			else
				return a.dimension.Width > b.dimension.Width;
		});
}

void blocklist::operator+=(blocklist& other)
{
	for (const tex_block& b : other.blocks)
		this->blocks.push_back(b);

	for (int n : other.material_groups)
		this->material_groups.push_back(n);
}

struct block_organizer
{
	#define MIN_BLOCK_SIZE 8

	struct column
	{
		int width;
		int length;
		vector<tex_block> blocks;
	};

	int dimension;
	int max_dimension;
	std::vector<column> columns;

	block_organizer(int initial_dimension, int max_dimension)
		:dimension(initial_dimension), max_dimension(max_dimension)
	{
		for(int d = dimension; d >= MIN_BLOCK_SIZE; d /= 2)
			columns.push_back(column{ d, 0 });
	}

	int current_dimension()
	{
		return dimension;
	}

	int current_size()
	{
		int size = 0;
		for (column& c : columns)
			size += c.length * c.width;
		return size;
	}

	int current_capacity()
	{
		return (dimension * dimension) - current_size();
	}

	int max_capacity()
	{
		return (max_dimension * max_dimension) - current_size();
	}

	void increase_dimension(int new_dimension)
	{
		if (new_dimension <= dimension)
			return;

		for (int d = dimension; d < new_dimension; d*=2)
		{
			columns.push_back(column{ d*2, 0 });
		}

		sort(columns.begin(), columns.end(),
			[&](column& a, column& b)
			{return a.width > b.width; });

		dimension = new_dimension;
		max_dimension = max(max_dimension, new_dimension);
	}

	bool can_fit(tex_block& b)
	{
		return (b.dimension.Width <= dimension && b.dimension.Height <= dimension &&
			b.dimension.Width >= MIN_BLOCK_SIZE &&
			(b.dimension.Width * b.dimension.Height) < current_capacity());
	}

	bool add(tex_block& b)
	{
		if (b.dimension.Width <= dimension && b.dimension.Height <= dimension &&
			b.dimension.Width >= MIN_BLOCK_SIZE &&
			(b.dimension.Width * b.dimension.Height) <= current_capacity())
		{
			for (column& c : columns)
			{
				if (c.width == b.dimension.Width)
				{
					c.length += b.dimension.Height;
					c.blocks.push_back(b);

					std::sort(c.blocks.begin(), c.blocks.end(),
						[&](tex_block& a, tex_block& b)
						{return a.dimension.Height > b.dimension.Height; });

					break;
				}
			}
			return true;
		}
		return false;
	}

	struct subdivision
	{
		dimension2du dimension;
		rect<u16> coords;
		subdivision* divs[2] = { NULL,NULL };

		subdivision(dimension2du dim) : dimension(dim),
			coords{0,0,(u16)dim.Width,(u16)dim.Height}
		{}

		~subdivision()
		{
			if (divs[0])
				delete divs[0];

			if (divs[1])
				delete divs[1];
		}

		bool hasSubDivisions()
		{
			return divs[0] && divs[1];
		}

		void subdivide(u16 len)
		{
			if (hasSubDivisions())
			{
				std::cout << "error\n";
				return;
			}
			divs[0] = new subdivision(dimension2du(len, dimension.Height));
			divs[0]->coords = rect<u16>(coords.UpperLeftCorner,
				vector2d<u16>(coords.UpperLeftCorner.X + len, coords.LowerRightCorner.Y));

			divs[1] = new subdivision(dimension2du(dimension.Width - len, dimension.Height));
			divs[1]->coords = rect<u16>(vector2d<u16>(coords.UpperLeftCorner.X + len, coords.UpperLeftCorner.Y),
				coords.LowerRightCorner);
		}

		bool try_add(tex_block& r)
		{
			if (r.dimension.Width > dimension.Width ||
				r.dimension.Height > dimension.Height)
			{
				return false;
			}

			if (hasSubDivisions())
			{
				if (divs[0]->try_add(r))
					return true;
				else
					return divs[1]->try_add(r);
			}

			if (r.dimension.Width < dimension.Width)
			{
				subdivide(r.dimension.Width);

				return divs[0]->try_add(r);
			}

			r.coords.UpperLeftCorner = coords.UpperLeftCorner;
			r.coords.LowerRightCorner = vector2d<u16>(coords.UpperLeftCorner.X + r.dimension.Width,
				coords.UpperLeftCorner.Y + r.dimension.Height);

			dimension.Height -= r.dimension.Height;
			coords.UpperLeftCorner.Y += r.dimension.Height;

			return true;
		}
	};

	

	void calc_coords(blocklist& out)
	{
		subdivision div_0(dimension2du{ (u16)dimension,(u16)dimension });

		for (column& c : columns)
		{
			for (tex_block& b : c.blocks)
				div_0.try_add(b);
		}

		out.blocks.clear();
		for (column& c : columns)
		{
			for (tex_block& b : c.blocks)
				out.blocks.push_back(b);
		}
	}

};


void initialize_lightmap_block(GeometryStack* geo_node, int element_id, int surface_no, std::back_insert_iterator<std::vector<Lightmap_Block>> ret, int reduce)
{
	geo_element* element = geo_node->get_element_by_id(element_id);
	polyfold* pf = &geo_node->get_element_by_id(element_id)->brush;

	matrix4 m_translate;
	matrix4 m_scale;

	vector<int> surface;

	for (int f_i : element->surfaces[surface_no].my_faces)
	{
		if (geo_node->get_element_by_id(element_id)->geometry.faces[f_i].loops.size() > 0)
			surface.push_back(f_i);
	}

	if (surface.size() == 0)
		return;

	surface_group& sfg = *pf->getFaceSurfaceGroup(surface[0]);

	{
		Lightmap_Block b;
		b.element_id = element_id;
		b.surface_no = surface_no;

		for (int f_j : surface)
		{
			pf->faces[f_j].temp_b = true;
		}

		//if (element->surfaces[surface_no].lightmap_info.bOverrideSize == false)
		{
			switch (element->surfaces[surface_no].surface_type)
			{
			case SURFACE_GROUP_STANDARD:
			case SURFACE_GROUP_CUSTOM_UVS_BRUSH:
			case SURFACE_GROUP_CUSTOM_UVS_GEOMETRY:
			{
				pf->calc_tangent(surface[0]);

				poly_face* f = &pf->faces[surface[0]];
				b.width = reduce_dimension_base2(f->bbox2d.getWidth(), reduce);
				b.height = reduce_dimension_base2(f->bbox2d.getHeight(), reduce);

				rectf new_box;
				new_box.UpperLeftCorner.X = f->bbox2d.LowerRightCorner.Y;
				new_box.UpperLeftCorner.Y = f->bbox2d.UpperLeftCorner.X;
				new_box.LowerRightCorner.X = f->bbox2d.UpperLeftCorner.Y;
				new_box.LowerRightCorner.Y = f->bbox2d.LowerRightCorner.X;

				f->bbox2d = new_box;
				f->m_tangent = f->m_normal.crossProduct(f->m_tangent);

				b.faces = surface;

			} break;

			case SURFACE_GROUP_CANONICAL:
			case SURFACE_GROUP_CYLINDER:
			{
				b.width = reduce_dimension_base2(sfg.c_brush.get_real_length(pf), reduce);
				b.height = reduce_dimension_base2(sfg.c_brush.get_real_height(pf), reduce);

				b.faces = surface;

			} break;
			case SURFACE_GROUP_DOME:
			case SURFACE_GROUP_SPHERE:
			{
				b.width = reduce_dimension_base2(sfg.radius * 2 * 3.1459 * 2, reduce);
				b.height = b.width;
				b.faces = surface;

				for (int b_i : surface)
				{
					pf->calc_tangent(b_i);

					if (b.height > b.width)
					{
						poly_face* f = &pf->faces[b_i];
						rectf new_box;
						new_box.UpperLeftCorner.X = f->bbox2d.LowerRightCorner.Y;
						new_box.UpperLeftCorner.Y = f->bbox2d.UpperLeftCorner.X;
						new_box.LowerRightCorner.X = f->bbox2d.UpperLeftCorner.Y;
						new_box.LowerRightCorner.Y = f->bbox2d.LowerRightCorner.X;

						f->bbox2d = new_box;
						f->m_tangent = f->m_normal.crossProduct(f->m_tangent);
						f->m_tangent *= -1;
					}
				}
			} break;
			default:
			{
				std::cout << "error, unknown surface type...\n";
				return;
			}break;
			}

			if (b.height > b.width)
			{
				swap(b.width, b.height);
				b.bFlipped = true;
			}
		}

		//if (element->surfaces[surface_no].lightmap_info.bOverrideSize == true)
		if(false)
		{
			b.bFlipped = element->surfaces[surface_no].lightmap_info.bFlipped;
			b.height = element->surfaces[surface_no].lightmap_info.height;
			b.width = element->surfaces[surface_no].lightmap_info.width;
			b.faces = surface;
		}
		else
		{
			element->surfaces[surface_no].lightmap_info.height = b.height;
			element->surfaces[surface_no].lightmap_info.width = b.width;
			element->surfaces[surface_no].lightmap_info.bFlipped = b.bFlipped;
		}

		*ret = b;
		++ret;
	}
}

void Lightmap_Configuration::calc_lightmap_uvs(const std::vector<TextureMaterial>& material_groups)
{
	materials = material_groups;

	for (int i = 0; i < materials.size(); i++)
	{
		if (!materials[i].has_lightmap)
			continue;

		for (int j = 0; j < materials[i].blocks.size(); j++)
		{
			Lightmap_Block& b = materials[i].blocks[j];

			if (reduce_power != 0)
			{
				b.height = reduce_dimension_base2(b.height, reduce_power);
				b.width = reduce_dimension_base2(b.width, reduce_power);
			}

			//to the raw lm uvs array
			calc_lightmap_uvs(geo_node,b);
		}
	}
	int z = 0;
}

void Lightmap_Configuration::calc_lightmap_uvs(GeometryStack* geo_node, Lightmap_Block b)
{
	geo_element* element = geo_node->get_element_by_id(b.element_id);
	polyfold* pf = &geo_node->get_element_by_id(b.element_id)->brush;
	//int(*length)(int) = []();

	int f_i = b.faces[0];
	surface_group& sfg = *pf->getFaceSurfaceGroup(f_i);
	int sfg_i = pf->getFaceSurfaceGroupNo(f_i);

	int offset = element->surfaces[b.surface_no].face_index_offset; //faces in lightmap block are local to the element
																	//and need to have an offset added

	switch (element->surfaces[b.surface_no].surface_type)
	{
	case SURFACE_GROUP_STANDARD:
	case SURFACE_GROUP_CUSTOM_UVS_BRUSH:
	case SURFACE_GROUP_CUSTOM_UVS_GEOMETRY:
	{
		map_face_to_uvs mapper;

		mapper.init(&pf->faces[b.faces[0]], 0, b.width, b.height);

		map_uvs(&geo_node->edit_meshnode_interface, offset, vector<int>{ b.faces[0] }, mapper, MAP_UVS_LIGHTMAP);

	} break;

	case SURFACE_GROUP_CYLINDER:
	case SURFACE_GROUP_CANONICAL:
	{
		MeshNode_Interface_Edit* mesh_node = &geo_node->edit_meshnode_interface;

		sfg.c_brush.layout_uvs(b.width, b.height);

		for (int b_i : b.faces)
		{
			int f_j = mesh_node->get_buffer_index_by_face(offset + b_i);

			int index_offset = mesh_node->indices_soa.offset[f_j];
			int len = mesh_node->indices_soa.len[f_j];

			for (int i = index_offset; i < index_offset + len; i += 3)
			{
				aligned_vec3* pos = mesh_node->geometry_raw_vertices.data.data();
				aligned_vec3* uv = this->lm_raw_uvs.data.data();

				u16 idx0 = mesh_node->indices_soa.data[i].x;
				u16 idx1 = mesh_node->indices_soa.data[i + 1].x;
				u16 idx2 = mesh_node->indices_soa.data[i + 2].x;

				sfg.c_brush.map_point(pf, b_i, pos[idx0].V, uv[idx0]);
				sfg.c_brush.map_point(pf, b_i, pos[idx1].V, uv[idx1]);
				sfg.c_brush.map_point(pf, b_i, pos[idx2].V, uv[idx2]);
			}
		}
	} break;
	case SURFACE_GROUP_DOME:
	case SURFACE_GROUP_SPHERE:
	{

		map_sphere_to_uv mapper;

		mapper.init(&sfg, pf, b.faces, sfg_i, b.width, b.height, false);

		map_uvs(&geo_node->edit_meshnode_interface, offset, b.faces, mapper, MAP_UVS_LIGHTMAP);

	} break;
	default:
	{
		std::cout << "error, unknown surface type...\n";
		return;
	}break;
	}
}

void Lightmap_Configuration::transform_lightmap_uvs(MeshNode_Interface_Edit* edit_node, int element_no, int surface_no, const matrix4& mat)
{
	u16 offset_e = edit_node->surface_index.offset[element_no];

	for (int j = 0; j < geo_node->elements[element_no].surfaces[surface_no].my_faces.size(); j++)
	{
		
		int f = geo_node->elements[element_no].reverse_index[geo_node->elements[element_no].surfaces[surface_no].my_faces[j]];
		u16 offset_i = edit_node->get_buffer_index_by_face(f);

		for (u16 i = 0; i < edit_node->geometry_raw_vertices.len[offset_i]; i++)
		{
			u16 idx_e = edit_node->geometry_raw_vertices.offset[offset_i] + i;

			f32 V4[4] = { 0,0,0,1 };

			V4[0] = lm_raw_uvs.data.data()[idx_e].V.X;
			V4[1] = lm_raw_uvs.data.data()[idx_e].V.Y;

			mat.multiplyWith1x4Matrix(V4);

			lm_raw_uvs.data.data()[idx_e].V.X = V4[0];
			lm_raw_uvs.data.data()[idx_e].V.Y = V4[1];
		}
	}
}


void Lightmap_Configuration::copy_raw_lightmap_uvs_to_mesh(dimension2du lm_dimension, tex_block& block)
{
	MeshNode_Interface_Edit* mesh_node = &geo_node->edit_meshnode_interface;

	geo_element* e = geo_node->get_element_by_id(block.element_id);
	poly_surface* s = &e->surfaces[block.surface_no];

	std::vector<int> faces;

	for (int f : s->my_faces)
	{
		int f_i = f + s->face_index_offset;
		int f_j = geo_node->edit_meshnode_interface.get_buffer_index_by_face(f_i);
		if (f_j != -1)
			faces.push_back(f_j);
	}

	copy_raw_lightmap_uvs_to_mesh(mesh_node, faces);
}

void Lightmap_Configuration::transform_lightmap_uvs(dimension2du lm_dimension, tex_block& block)
{
	MeshNode_Interface_Edit* mesh_node = &geo_node->edit_meshnode_interface;
	int element_no = geo_node->element_by_element_id[block.element_id];

	matrix4 flip_mat   (0, 1, 0, 0,
						1, 0, 0, 0,
						0, 0, 0, 0,
						0, 0, 0, 0);

	matrix4 m;

	f32 x_factor = 1.0f / (f32)lm_dimension.Width;
	f32 y_factor = 1.0f / (f32)lm_dimension.Height;

	m.setScale(vector3df(x_factor, y_factor, 1.0f));

	m.setTranslation(vector3df((f32)(block.coords.UpperLeftCorner.X + 0.5) / (f32)lm_dimension.Width,
		(f32)(block.coords.UpperLeftCorner.Y + 0.5) / (f32)lm_dimension.Height, 0));
	
	if (block.bFlipped)
	{
		transform_lightmap_uvs(mesh_node, element_no,block.surface_no, flip_mat);
	}

	transform_lightmap_uvs(mesh_node, element_no, block.surface_no, m);
		
	u16 offset_e = mesh_node->surface_index.offset[element_no];
	u16 offset_i = mesh_node->get_buffer_index_by_face(offset_e + block.surface_no);
}


template<typename ret_type>
bool add_blocklists(ret_type ret, vector<blocklist>::iterator& it, const vector<blocklist>::iterator& end)
{
	blocklist out_list;

	block_organizer canvas(256, 256);

	for (; it != end; ++it)
	{
		if (it->size <= canvas.current_capacity())
		{
			for (tex_block& b : it->blocks)
			{
				canvas.add(b);
			}
		}
		else
		{
			canvas.calc_coords(out_list);
			*ret = out_list;
			return false;
		}

		out_list += *it;
	}

	canvas.calc_coords(out_list);
	*ret = out_list;
	return true;
}

template<typename out_type>
bool divide_material_groups(GeometryStack* geo_scene, out_type out, vector<Lightmap_Block>::iterator& blocks_it, vector<Lightmap_Block>::iterator& blocks_end, const TextureMaterial& copy_from)
{
	TextureMaterial ret;

	ret.lightmap_size = 256;
	ret.has_lightmap = true;

	//ret.original_material_no = copy_from.original_material_no;
	ret.materialGroup = copy_from.materialGroup;
	ret.texture = copy_from.texture;

	auto ret_faces_back = std::back_inserter(ret.faces);
	auto ret_surfaces_back = std::back_inserter(ret.surfaces);

	int total_size = 0;
	for (; blocks_it != blocks_end; ++blocks_it)
	{
		int block_size = blocks_it->height * blocks_it->width;

		if (total_size + block_size > 256 * 256)
		{
			*out = ret;
			return false;
		}
		total_size += block_size;

		ret.blocks.push_back(*blocks_it);

		int offset = geo_scene->get_element_by_id(blocks_it->element_id)->surfaces[blocks_it->surface_no].face_index_offset;
		
		for (int f_j : blocks_it->faces)
		{
			*ret_faces_back = offset + f_j;
			++ret_faces_back;
		}

		*ret_surfaces_back = std::pair<int, int>{ blocks_it->element_id, blocks_it->surface_no };
		++ret_surfaces_back;
	}

	*out = ret;
	return true;
}

void Lightmap_Configuration::split_material_groups()
{
	std::vector<TextureMaterial> new_materials;

	auto in_it = materials.begin();
	auto in_end = materials.end();

	auto materials_back = back_inserter(new_materials);

	for (; in_it != in_end; ++in_it)
	{
		if (in_it->has_lightmap == false)
		{
			*materials_back = *in_it;
			++materials_back;
			continue;
		}

		vector<Lightmap_Block> blocks = in_it->blocks;

		std::sort(blocks.begin(), blocks.end(),
			[&](Lightmap_Block& a, Lightmap_Block& b)
			{
				if (a.width == b.width)
				{
					return a.height > b.height;
				}
				else
					return a.width > b.width;
			});

		auto blocks_it = blocks.begin();
		auto blocks_end = blocks.end();

		while (!divide_material_groups(geo_node, materials_back, blocks_it, blocks_end, *in_it))
		{
			++materials_back;
		}

		++materials_back;
	}

	materials = new_materials;
}

void Lightmap_Configuration::layout_lightmaps()
{
	if (materials.size() == 0)
		return;
	
	vector<blocklist> blocklists;
	auto bl_back = back_inserter(blocklists);

	int mg_n = 0;
	for (const TextureMaterial& tm : materials)
	{
		blocklist bl;

		if (tm.has_lightmap == false)
		{
			mg_n++;
			continue;
		}

		for (const Lightmap_Block& lmb : tm.blocks)
		{
			bl.blocks.push_back(tex_block{ dimension2du(lmb.width, lmb.height), lmb.element_id, lmb.surface_no, lmb.bFlipped });
		}

		bl.material_groups = vector<int>{ mg_n };
		mg_n++;

		bl.sort();
		bl.calc_size();

		if (bl.size == 0)
			continue;

		*bl_back = bl;
		bl_back++;
	}
	cout << "\nblocklists: \n";

	std::sort(blocklists.begin(),blocklists.end(),
		[&](blocklist& a, blocklist& b)
		{
			return a.size < b.size;
		});

	//=============
	//Combine

	bl_combined.clear();

	auto blc_back = std::back_inserter(bl_combined);
	vector<blocklist>::iterator& bl_it = blocklists.begin();

	while (!add_blocklists(blc_back, bl_it, blocklists.end()))
	{
		++blc_back;
	}

	for (blocklist& b : bl_combined)
	{
		b.calc_size();
		cout << "size: " << b.size << ", ";
		cout << "material groups: ";
		for (int n : b.material_groups)
			cout << n << " ";
		cout << "\n";
	}

	lightmap_dimensions.clear();
	
	for (int i = 0; i < bl_combined.size(); i++)
	{
		lightmap_dimensions.push_back(dimension2du(256, 256));

		for (int n : bl_combined[i].material_groups)
		{
			materials[n].lightmap_no = i;
		}
	}

	for (TextureMaterial& tm : materials)
		tm.lightmap_size = 256;

	for (TextureMaterial& tm : materials)
	{
		tm.n_faces = tm.faces.size();

		int n_triangles = 0;

		if (tm.faces.size() > 0)
		{
			MeshBuffer_Chunk chunk = geo_node->final_meshnode_interface.get_mesh_buffer_by_face(tm.faces[0]);

			if (chunk.buffer)
				n_triangles = chunk.buffer->getIndexCount() / 3;
		}

		tm.n_triangles = n_triangles;
	}
}

void Lightmap_Configuration::transform_lightmap_uvs()
{
	for (blocklist& b : bl_combined)
	{
		for (tex_block& tb : b.blocks)
		{
			transform_lightmap_uvs(dimension2du(256, 256), tb);
		}
	}
}

void Lightmap_Configuration::apply_lightmap_uvs_to_mesh()
{
	//geo_node->final_meshnode_interface.face_to_material.assign(geo_node->final_meshnode_interface.face_to_mb_buffer.size(), -1);

	for (int i = 0; i < materials.size(); i++)
	{
		TextureMaterial& tm = materials[i];

		for (int f_i : tm.faces)
		{
			int f_j = geo_node->final_meshnode_interface.edit_mb_buffer[f_i];
			geo_node->edit_meshnode_interface.face_to_material[f_j] = i;
		}
	}

	for (blocklist& b : bl_combined)
	{
		for (tex_block& tb : b.blocks)
		{
			copy_raw_lightmap_uvs_to_mesh(dimension2du(256, 256), tb);
		}
	}

	geo_node->final_meshnode_interface.copy_lightmap_uvs();
}

void Lightmap_Configuration::set_reduction(int p)
{
	reduce_power = p;
}

void Lightmap_Configuration::initialize_soa_arrays(scene::SMesh* mesh)
{
	typedef CMeshBuffer<video::S3DVertex2TCoords> mesh_buffer_type;

	u32 n = mesh->getMeshBufferCount();

	//vertices
	{

		u32(*length)(SMesh*, u32);
		aligned_vec3(*item0)(SMesh*, u32, u32);

		length = [](SMesh* mesh_, u32 index) -> u32 {
			return ((mesh_buffer_type*)mesh_->getMeshBuffer(index))->getVertexCount();
			};

		item0 = [](SMesh* mesh_, u32 i, u32 j) -> aligned_vec3 {
			return aligned_vec3{ core::vector3df{0,0,0} };
			};

		lm_raw_uvs.fill_data(mesh, n, length, item0);
	}
	
}


void Lightmap_Configuration::copy_raw_lightmap_uvs_to_mesh(MeshNode_Interface_Edit* mesh_node, const std::vector<int>& surface)
{
	video::S3DVertex2TCoords* vtx[3];

	for (int b_i : surface)
	{
		MeshBuffer_Chunk chunk = mesh_node->get_mesh_buffer(b_i);
		int offset = mesh_node->indices_soa.offset[b_i];
		int len = mesh_node->indices_soa.len[b_i];
		//std::cout << b_i << ":\n";
		int j = chunk.begin_i;
		for (int i = offset; i < offset + len && j < chunk.end_i; i += 3, j += 3)
		{
			u16 idx0 = chunk.buffer->getIndices()[j];
			u16 idx1 = chunk.buffer->getIndices()[j + 1];
			u16 idx2 = chunk.buffer->getIndices()[j + 2];

			vtx[0] = &((video::S3DVertex2TCoords*)chunk.buffer->getVertices())[idx0];
			vtx[1] = &((video::S3DVertex2TCoords*)chunk.buffer->getVertices())[idx1];
			vtx[2] = &((video::S3DVertex2TCoords*)chunk.buffer->getVertices())[idx2];

			idx0 = mesh_node->indices_soa.data[i].x;
			idx1 = mesh_node->indices_soa.data[i + 1].x;
			idx2 = mesh_node->indices_soa.data[i + 2].x;

			vtx[0]->TCoords2.X = this->lm_raw_uvs.data[idx0].V.X;
			vtx[0]->TCoords2.Y = this->lm_raw_uvs.data[idx0].V.Y;

			vtx[1]->TCoords2.X = this->lm_raw_uvs.data[idx1].V.X;
			vtx[1]->TCoords2.Y = this->lm_raw_uvs.data[idx1].V.Y;

			vtx[2]->TCoords2.X = this->lm_raw_uvs.data[idx2].V.X;
			vtx[2]->TCoords2.Y = this->lm_raw_uvs.data[idx2].V.Y;

			//std::cout << " " << vtx[0]->TCoords2.X << "," << vtx[0]->TCoords2.Y << "\n";
			//std::cout << " " << vtx[1]->TCoords2.X << "," << vtx[1]->TCoords2.Y << "\n";
			//std::cout << " " << vtx[2]->TCoords2.X << "," << vtx[2]->TCoords2.Y << "\n";
		}
	}
}