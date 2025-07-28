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

using namespace irr;
using namespace core;
using namespace std;

Lightmap_Manager* Lightmaps_Tool::lightmap_manager = NULL;

extern IrrlichtDevice* device;


Lightmap_Manager::Lightmap_Manager()
{
	MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
	event_receiver = receiver;
}

void Lightmap_Manager::loadLightmapTextures(geometry_scene* geo_scene)
{
	GeometryStack* geo_node = geo_scene->geoNode();
	std::vector<TextureMaterial> material_groups = geo_node->final_meshnode_interface.getMaterialsUsed();

	SEvent event;
	event.EventType = EET_USER_EVENT;
	event.UserEvent.UserData1 = USER_EVENT_CLEAR_LIGHTMAP_TEXTURES;
	event_receiver->OnEvent(event);

	for (video::ITexture* tex : lightmap_textures)
	{
		device->getVideoDriver()->removeTexture(tex);
	}

	lightmap_textures.clear();

	int N_lightmaps = 0;
	for (int i = 0; i < material_groups.size(); i++)
	{
		N_lightmaps = std::max(N_lightmaps, material_groups[i].lightmap_no);
	}
	N_lightmaps += 1;

	lightmap_textures.resize(N_lightmaps);

	for (int i = 0; i < material_groups.size(); i++)
	{
		if (material_groups[i].has_lightmap == false)
		{
			//lightmap_textures.push_back(NULL);
			continue;
		}

		std::stringstream ss;

		io::path p = File_Open_Tool::get_base()->GetCurrentProjectPath();

		ss << p.c_str();
		ss << "/lightmap_" << geo_scene->get_unique_id() << "_" << material_groups[i].lightmap_no << ".bmp";

		//ss << "../projects/export/lightmap_" << inc << ".bmp";
		video::ITexture* tex = device->getVideoDriver()->getTexture(ss.str().c_str());

		//lightmap_textures.push_back(tex);
		lightmap_textures[material_groups[i].lightmap_no] = tex;

		for (int f_i : material_groups[i].faces)
		{
			MeshBuffer_Chunk chunk;
			chunk = geo_node->final_meshnode_interface.get_mesh_buffer_by_face(f_i);
			chunk.buffer->getMaterial().setTexture(1, tex);

			chunk = geo_node->edit_meshnode_interface.get_mesh_buffer_by_face(f_i);
			chunk.buffer->getMaterial().setTexture(1, tex);
		}
	}

	geo_node->getMeshNode()->copyMaterials();

	event.EventType = EET_USER_EVENT;
	event.UserEvent.UserData1 = USER_EVENT_MATERIAL_GROUP_SELECTION_CHANGED;
	event_receiver->OnEvent(event);
}

f32 reduce_dimension_base2(f32 dim, int n = 1)
{
	double log_width = log2(dim);
	u16 lwi = static_cast<u16>(floor(log_width));
	lwi -= n;
	lwi = lwi > 3 ? lwi : 3;
	lwi = lwi > 6 ? 6 : lwi;

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

	void calc_size()
	{
		size = 0;

		for (tex_block& b : blocks)
		{
			size += b.dimension.Width * b.dimension.Height;
		}
	}

	void sort()
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

	void operator+=(blocklist& other)
	{
		for (const tex_block& b : other.blocks)
			this->blocks.push_back(b);

		for (int n : other.material_groups)
			this->material_groups.push_back(n);
	}
};

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
/*
struct texture_block
{
	dimension2du dimension;
	int max_size = 512;
	int element_id = -1;
	int surface_no = -1;
	bool bFlipped = false;

	rect<u16> coords;

	texture_block* column[2] = { NULL,NULL};

	texture_block(int size)
	{
		dimension.Width = size;
		dimension.Height = size;
		coords.UpperLeftCorner = vector2d<u16>(0, 0);
		coords.LowerRightCorner = vector2d<u16>(size, size);
	}

	texture_block(dimension2du dim)
	{
		dimension = dim;
	}

	texture_block(int e, int s, bool flip, dimension2du dim)
	{
		dimension = dim;
		element_id = e;
		surface_no = s;
		bFlipped = flip;
	}

	~texture_block()
	{
		for (int i = 0; i < 2; i++)
			if (column[i])
				delete column[i];
	}

	bool hasSubColumns()
	{
		return column[0] && column[1];
	}

	void subdivide(u16 len)
	{
		if (hasSubColumns())
		{
			std::cout << "error\n";
			return;
		}
		column[0] = new texture_block(dimension2du(len, dimension.Height));
		column[0]->coords = rect<u16>(coords.UpperLeftCorner,
			vector2d<u16>(coords.UpperLeftCorner.X + len, coords.LowerRightCorner.Y));

		column[1] = new texture_block(dimension2du(dimension.Width - len, dimension.Height));
		column[1]->coords = rect<u16>(vector2d<u16>(coords.UpperLeftCorner.X + len, coords.UpperLeftCorner.Y),
			coords.LowerRightCorner);
	}

	void dump(int tab, int n)
	{
		for (int i = 0; i < tab; i++)
			std::cout << " ";

		std::cout << n << ": ";

		std::cout << "size=" << dimension.Width << "," << dimension.Height<<"  (";
		std::cout << coords.UpperLeftCorner.X << "," << coords.UpperLeftCorner.Y << " / ";
		std::cout << coords.LowerRightCorner.X << "," << coords.LowerRightCorner.Y << ")\n";

		for (int i = 0; i < 2; i++)
		{
			if (column[i])
			{
				column[i]->dump(tab + 1, i);
			}
		}
	}

	bool try_add(texture_block& r)
	{
		if (r.dimension.Width > dimension.Width ||
			r.dimension.Height > dimension.Height)
		{
			return false;
		}

		if (hasSubColumns())
		{
			if (column[0]->try_add(r))
				return true;
			else
				return column[1]->try_add(r);
		}

		if (r.dimension.Width < dimension.Width)
		{
			subdivide(r.dimension.Width);

			return column[0]->try_add(r);
		}

		r.coords.UpperLeftCorner = coords.UpperLeftCorner;
		r.coords.LowerRightCorner = vector2d<u16>(coords.UpperLeftCorner.X + r.dimension.Width, 
			coords.UpperLeftCorner.Y +r.dimension.Height);

		dimension.Height -= r.dimension.Height;
		coords.UpperLeftCorner.Y += r.dimension.Height;

		return true;
	}
};
*/

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

		/*
		surface_group& sfg = *pf->getFaceSurfaceGroup(f_i);
		int sfg_i = pf->getFaceSurfaceGroupNo(f_i);

		switch (sfg.type)
		{
		case SURFACE_GROUP_CYLINDER:
		case SURFACE_GROUP_DOME:
		case SURFACE_GROUP_SPHERE:
			surface = pf->getSurfaceFromFace(f_i);
			break;
		default:
			surface = vector<int>{ f_i };
			break;
		}*/

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
				//b.bounding_verts_index0 = 1;

				rectf new_box;
				new_box.UpperLeftCorner.X = f->bbox2d.LowerRightCorner.Y;
				new_box.UpperLeftCorner.Y = f->bbox2d.UpperLeftCorner.X;
				new_box.LowerRightCorner.X = f->bbox2d.UpperLeftCorner.Y;
				new_box.LowerRightCorner.Y = f->bbox2d.LowerRightCorner.X;

				f->bbox2d = new_box;
				f->m_tangent = f->m_normal.crossProduct(f->m_tangent);

				b.faces = surface;

			} break;

			case SURFACE_GROUP_CYLINDER:
			{
				b.width = reduce_dimension_base2(sfg.radius * 2 * 3.1459 * 2, reduce);
				b.height = reduce_dimension_base2(sfg.height, reduce);
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

		if (element->surfaces[surface_no].lightmap_info.bOverrideSize == true)
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

void calc_lightmap_uvs(GeometryStack* geo_node, Lightmap_Block b)
{
	geo_element* element = geo_node->get_element_by_id(b.element_id);
	polyfold* pf = &geo_node->get_element_by_id(b.element_id)->brush;


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
	{
		map_sphere_to_uv mapper;

		mapper.init(&sfg, pf, b.faces, sfg_i, b.width, b.height, true);

		map_uvs(&geo_node->edit_meshnode_interface, offset, b.faces, mapper, MAP_UVS_LIGHTMAP);

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

void set_lightmap_uvs_to_mesh(GeometryStack* geo_stack, dimension2du lm_dimension, tex_block& block)
{
	MeshNode_Interface_Edit* mesh_node = &geo_stack->edit_meshnode_interface;

	geo_element* e = geo_stack->get_element_by_id(block.element_id);
	poly_surface* s = &e->surfaces[block.surface_no];
	
	std::vector<int> faces = s->my_faces;

	for (int& f : faces)
		f += s->face_index_offset;

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

	copy_raw_lightmap_uvs_to_mesh(mesh_node, faces);

	if (block.bFlipped)
	{
		apply_transform_to_uvs(mesh_node, faces, MAP_UVS_LIGHTMAP, flip_mat);
	}

	apply_transform_to_uvs(mesh_node, faces, MAP_UVS_LIGHTMAP, m);

}
/*
template<typename out_Type>
bool lightmaps_fill(GeometryStack* geo_stack, std::vector<Lightmap_Block>::iterator& blocks_it, std::vector<Lightmap_Block>::iterator blocks_end, out_Type out, const TextureMaterial& copy_from)
{
	MeshNode_Interface_Edit* mesh_node = &geo_stack->edit_meshnode_interface;

	texture_block new_block(256);
	texture_block face_block(128);

	TextureMaterial ret;

	ret.materialGroup = copy_from.materialGroup;
	ret.texture = copy_from.texture;

	auto ret_faces_back = std::back_inserter(ret.my_faces);
	auto ret_surfaces_back = std::back_inserter(ret.surfaces);

	rect<u16> my_block;

	matrix4 flip_mat   (0, 1, 0, 0,
						1, 0, 0, 0,
						0, 0, 0, 0,
						0, 0, 0, 0);
	
	for (; blocks_it != blocks_end; ++blocks_it)
	{
		face_block.dimension = dimension2du(blocks_it->width, blocks_it->height);

		if (!new_block.try_add(face_block))
		{
			*out = ret;
			return false;
		}

		u32 block_height = 256;
		u32 block_width = 256;

		std::vector<int> faces = blocks_it->faces;

		for (int& f_i : faces)
			f_i = geo_stack->get_element_by_id(blocks_it->element_id)->reverse_index[f_i];

		matrix4 m;

		f32 x_factor = 1.0f / (f32)block_width;
		f32 y_factor = 1.0f / (f32)block_height;

		m.setScale(vector3df(x_factor,y_factor,1.0f));

		m.setTranslation(vector3df((f32)(face_block.coords.UpperLeftCorner.X + 0.5) / (f32)block_width,
			(f32)(face_block.coords.UpperLeftCorner.Y + 0.5) / (f32)block_height, 0));

		copy_raw_lightmap_uvs_to_mesh(mesh_node, faces);

		if (blocks_it->bFlipped)
		{
			apply_transform_to_uvs(mesh_node, faces, MAP_UVS_LIGHTMAP, flip_mat);
		}

		apply_transform_to_uvs(mesh_node, faces, MAP_UVS_LIGHTMAP, m);

		for (int f_j : blocks_it->faces)
		{
			*ret_faces_back = face_index{ blocks_it->element_id, f_j };
			++ret_faces_back;
		}
		
		*ret_surfaces_back = std::pair<int, int>{ blocks_it->element_id, blocks_it->surface_no };
		++ret_surfaces_back;
		
		ret.lightmap_size = new_block.dimension.Width;
		ret.has_lightmap = true;
	}

	*out = ret;

	return true;
}*/

/*
void lightmaps_divideMaterialGroups(GeometryStack* geo_scene, std::vector<TextureMaterial>& material_groups)
{
	std::vector<TextureMaterial> ret;
	auto ret_back = std::back_inserter(ret);

	auto in_it = material_groups.begin();
	auto in_end = material_groups.end();

	polyfold* pf = geo_scene->get_total_geometry();

	for (; in_it != in_end; ++in_it)
	{
		if (in_it->has_lightmap == false)
		{
			*ret_back = *in_it;
			++ret_back;
			continue;
		}
		
		std::sort(in_it->blocks.begin(), in_it->blocks.end(),
			[&](Lightmap_Block& b_a, Lightmap_Block& b_b)
			{
				if (b_a.width == b_b.width)
				{
					return b_a.height > b_b.height;
				}
				else
					return b_a.width > b_b.width;
			});

		auto blocks_it = in_it->blocks.begin();
		auto blocks_end = in_it->blocks.end();

		while (!lightmaps_fill(geo_scene, blocks_it, blocks_end, ret_back, *in_it))
		{
			++ret_back;
		}

		++ret_back;
	}
	
	std::cout << "Created "<<ret.size() << " new material groups\n";

	material_groups = ret;
}*/



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
bool divide_material_groups(GeometryStack* geo_scene, out_type out, vector<Lightmap_Block>::iterator& blocks_it, vector<Lightmap_Block>::iterator& blocks_end, TextureMaterial& copy_from)
{
	TextureMaterial ret;

	ret.lightmap_size = 256;
	ret.has_lightmap = true;

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
			*ret_faces_back =  offset + f_j;
			++ret_faces_back;
		}

		*ret_surfaces_back = std::pair<int, int>{ blocks_it->element_id, blocks_it->surface_no };
		++ret_surfaces_back;
	}

	*out = ret;
	return true;
}

void split_material_groups(GeometryStack* geo_scene, std::vector<TextureMaterial>& material_groups_in)
{
	if (material_groups_in.size() == 0)
		return;

	auto in_it = material_groups_in.begin();
	auto in_end = material_groups_in.end();

	vector<TextureMaterial> material_groups;
	auto materials_back = back_inserter(material_groups);

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

		while (!divide_material_groups(geo_scene, materials_back, blocks_it, blocks_end, *in_it))
		{
			++materials_back;
		}

		++materials_back;
	}

	material_groups_in = material_groups;
}

void layout_lightmaps(GeometryStack* geo_scene, vector<TextureMaterial>& material_groups)
{
	if (material_groups.size() == 0)
		return;
	
	vector<blocklist> blocklists;
	auto bl_back = back_inserter(blocklists);

	int mg_n = 0;
	for (TextureMaterial& tm : material_groups)
	{
		blocklist bl;

		if (tm.has_lightmap == false)
		{
			mg_n++;
			continue;
		}

		for (Lightmap_Block& lmb : tm.blocks)
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

	vector<blocklist> bl_combined;

	auto blc_back = std::back_inserter(bl_combined);
	vector<blocklist>::iterator& bl_it = blocklists.begin();

	while (!add_blocklists(blc_back, bl_it, blocklists.end()))
	{
		++blc_back;
	}

	for (blocklist& b : bl_combined)
	{
		b.calc_size();
		cout << "size: " << b.size << "\n";
		//cout << "faces: ";
		//for (tex_block& tb : b.blocks)
		//	cout << tb.id << " ";
		cout << "material groups: ";
		for (int n : b.material_groups)
			cout << n << " ";
		cout << "\n";
	}
	
	for (int i = 0; i < bl_combined.size(); i++)
	{
		for (int n : bl_combined[i].material_groups)
		{
			material_groups[n].lightmap_no = i;
		}
	}

	for (TextureMaterial& tm : material_groups)
		tm.lightmap_size = 256;
	/*
	std::sort(material_groups.begin(), material_groups.end(),
		[&](TextureMaterial& a, TextureMaterial& b)
		{
			return a.lightmap_no < b.lightmap_no;
		});*/

	for (blocklist& b : bl_combined)
	{
		for (tex_block& tb : b.blocks)
			set_lightmap_uvs_to_mesh(geo_scene, dimension2du(256, 256), tb);
	}

}
