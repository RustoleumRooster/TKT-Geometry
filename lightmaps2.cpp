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

using namespace irr;
using namespace core;
using namespace std;

extern IrrlichtDevice* device;

f32 reduce_dimension_base2(f32 dim, int n = 1)
{
	double log_width = log2(dim);
	u16 lwi = static_cast<u16>(floor(log_width));
	lwi -= n;
	lwi = lwi > 3 ? lwi : 3;
	lwi = lwi > 7 ? 7 : lwi;

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

struct texture_block2
{
	dimension2du dimension;
	int max_size = 512;

	rect<u16> coords;

	texture_block2* column[2] = { NULL,NULL};

	texture_block2(int size)
	{
		dimension.Width = size;
		dimension.Height = size;
		coords.UpperLeftCorner = vector2d<u16>(0, 0);
		coords.LowerRightCorner = vector2d<u16>(size, size);
	}

	texture_block2(dimension2du dim)
	{
		dimension = dim;
	}

	~texture_block2()
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
		column[0] = new texture_block2(dimension2du(len, dimension.Height));
		column[0]->coords = rect<u16>(coords.UpperLeftCorner,
			vector2d<u16>(coords.UpperLeftCorner.X + len, coords.LowerRightCorner.Y));

		column[1] = new texture_block2(dimension2du(dimension.Width - len, dimension.Height));
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

	bool try_add(texture_block2& r)
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

struct lm_block
{
	std::vector<int> faces;
	u32 width = 128;
	u32 height = 128;
	bool bFlipped = false;
	int bounding_verts_index0 = 0;
};

template<typename out_Type>
bool lightmaps_fill(MeshNode_Interface* mesh_node, std::vector<lm_block>::iterator& blocks_it, std::vector<lm_block>::iterator blocks_end, out_Type out, const TextureMaterial& copy_from)
{
	texture_block2 new_block(256);
	texture_block2 face_block(128);

	TextureMaterial ret;

	ret.materialGroup = copy_from.materialGroup;
	ret.texture = copy_from.texture;

	auto ret_faces_back = std::back_inserter(ret.faces);

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

		matrix4 m;

		/*
		std::cout << " scale = " << (f32)(blocks_it->width - 3) / (f32)block_width << " / " <<
			(f32)(blocks_it->width - 3) / (f32)block_width << "\n";

		std::cout<<" trans = " << (f32)(face_block.coords.UpperLeftCorner.X + 1.5f) / (f32)block_width << " / " <<
			(f32)(face_block.coords.UpperLeftCorner.Y + 1.5f) / (f32)block_height << "\n";
		*/

		//m.setScale(vector3df((f32)(blocks_it->width - 3) / (f32)block_width,
		//	(f32)(blocks_it->height - 3) / (f32)block_height, 1.0f));

		//m.setTranslation(vector3df((f32)(face_block.coords.UpperLeftCorner.X + 1.5f) / (f32)block_width,
		//	(f32)(face_block.coords.UpperLeftCorner.Y + 1.5f) / (f32)block_height, 0));
		//std::cout << blocks_it->width << " / " << block_width << "\n";

		f32 x_factor = 1.0f / (f32)block_width;
		f32 y_factor = 1.0f / (f32)block_height;

		m.setScale(vector3df(x_factor,y_factor,1.0f));

		m.setTranslation(vector3df((f32)(face_block.coords.UpperLeftCorner.X + 0.5) / (f32)block_width,
			(f32)(face_block.coords.UpperLeftCorner.Y + 0.5) / (f32)block_height, 0));

		
		if (blocks_it->bFlipped)
		{
			apply_transform_to_uvs(mesh_node, blocks_it->faces, MAP_UVS_LIGHTMAP, flip_mat);
		}

		apply_transform_to_uvs(mesh_node, blocks_it->faces, MAP_UVS_LIGHTMAP, m);

		for (int f_j : blocks_it->faces)
		{
			*ret_faces_back = f_j;
			++ret_faces_back;
		}
		
		ret.lightmap_size = new_block.dimension.Width;
		//ret.
	}

	*out = ret;

	return true;
}
/*
int is_reducible(geometry_scene* geo_scene, int f_i, int reduce)
{
	polyfold* pf = geo_scene->get_total_geometry();


	if (pf->faces[f_i].loops.size() > 0 && pf->faces[f_i].temp_b == false)
	{
		lm_block b;

		vector<int> surface;
		surface_group& sfg = *pf->getFaceSurfaceGroup(f_i);
		int sfg_i = pf->getFaceSurfaceGroupNo(f_i);

		switch (sfg.type)
		{
		case SURFACE_GROUP_CYLINDER:
		case SURFACE_GROUP_DOME:
		case SURFACE_GROUP_SPHERE:
			surface = geo_scene->getSurfaceFromFace(f_i);
			break;
		default:
			surface = vector<int>{ f_i };
			break;
		}

		for (int f_j : surface)
		{
			pf->faces[f_j].temp_b = true;
		}

		switch (sfg.type)
		{
			case SURFACE_GROUP_STANDARD:
			case SURFACE_GROUP_CUSTOM_UVS_BRUSH:
			case SURFACE_GROUP_CUSTOM_UVS_GEOMETRY:
			{
				map_face_to_uvs2 mapper;

				pf->calc_tangent(f_i);

				poly_face* f = &pf->faces[f_i];
				b.width = reduce_dimension_base2(f->bbox2d.getWidth(), reduce);
				b.height = reduce_dimension_base2(f->bbox2d.getHeight(), reduce);

				mapper.init(&geo_scene->get_total_geometry()->faces[f_i], 1, b.width, b.height);

				map_uvs(geo_scene, &geo_scene->edit_meshnode_interface, vector<int>{ f_i }, mapper, MAP_UVS_LIGHTMAP);

				b.faces = vector<int>{ f_i };



			} break;

			case SURFACE_GROUP_CYLINDER:
			{
				map_cylinder_to_uv2 mapper;

				b.width = reduce_dimension_base2(sfg.radius * 2 * 3.1459, reduce);
				b.height = reduce_dimension_base2(sfg.height, reduce);
				b.faces = surface;


			

			} break;
			case SURFACE_GROUP_DOME:
			case SURFACE_GROUP_SPHERE:
			{

				map_sphere_to_uv2 mapper;

				b.width = reduce_dimension_base2(sfg.radius * 2 * 3.1459 * 2, reduce);
				b.height = b.width;
				b.faces = surface;

				mapper.init(&sfg, pf, surface, sfg_i, b.width, b.height);

				for (int b_i : surface)
				{
					pf->calc_tangent(b_i);
					//cout << pf->faces[b_i].m_tangent.X << "," << pf->faces[b_i].m_tangent.Y << "," << pf->faces[b_i].m_tangent.Z << "\n";
				}

				map_uvs(geo_scene, &geo_scene->edit_meshnode_interface, surface, mapper, MAP_UVS_LIGHTMAP);

				

			} break;
			
			default:
			{
				std::cout << "error, unknown surface type...\n";
				return;
			}break;

		}

	}

}
*/

template<typename back_type>
void initialize_block(geometry_scene* geo_scene, int f_i, back_type ret, int reduce)
{
	polyfold* pf = geo_scene->get_total_geometry();

	matrix4 m_translate;
	matrix4 m_scale;

	vector<int> surface;

	if (pf->faces[f_i].loops.size() > 0 && pf->faces[f_i].temp_b == false)
	{
		lm_block b;

		
		surface_group& sfg = *pf->getFaceSurfaceGroup(f_i);
		int sfg_i = pf->getFaceSurfaceGroupNo(f_i);

		switch (sfg.type)
		{
			case SURFACE_GROUP_CYLINDER:
			case SURFACE_GROUP_DOME:
			case SURFACE_GROUP_SPHERE:
				surface = geo_scene->getSurfaceFromFace(f_i);
				break;
			default:
				surface = vector<int>{ f_i };
				break;
		}

		for (int f_j : surface)
		{
			pf->faces[f_j].temp_b = true;
		}

		/*
		surface = vector<int>{ f_i };
		for (int f_j : surface)
		{
			pf->faces[f_j].temp_b = true;
		}

		{
			map_face_to_uvs2 mapper;

			pf->calc_tangent(f_i);

			poly_face* f = &pf->faces[f_i];
			b.width = reduce_dimension_base2(f->bbox2d.getWidth(), reduce);
			b.height = reduce_dimension_base2(f->bbox2d.getHeight(), reduce);
			b.bounding_verts_index0 = 1;

			rectf new_box;
			new_box.UpperLeftCorner.X = f->bbox2d.LowerRightCorner.Y;
			new_box.UpperLeftCorner.Y = f->bbox2d.UpperLeftCorner.X;
			new_box.LowerRightCorner.X = f->bbox2d.UpperLeftCorner.Y;
			new_box.LowerRightCorner.Y = f->bbox2d.LowerRightCorner.X;

			f->bbox2d = new_box;
			f->m_tangent = f->m_normal.crossProduct(f->m_tangent);

			//mapper.init(&geo_scene->get_total_geometry()->faces[f_i], 1,b.width,b.height);
			mapper.init(&geo_scene->get_total_geometry()->faces[f_i], 0, b.width, b.height);

			map_uvs(geo_scene, &geo_scene->edit_meshnode_interface, vector<int>{ f_i }, mapper, MAP_UVS_LIGHTMAP);

			b.faces = vector<int>{ f_i };
		}*/

		switch (sfg.type)
		{
			case SURFACE_GROUP_STANDARD:
			case SURFACE_GROUP_CUSTOM_UVS_BRUSH:
			case SURFACE_GROUP_CUSTOM_UVS_GEOMETRY:
			{
				map_face_to_uvs2 mapper;

				pf->calc_tangent(f_i);

				poly_face* f = &pf->faces[f_i];
				b.width = reduce_dimension_base2(f->bbox2d.getWidth(), reduce);
				b.height = reduce_dimension_base2(f->bbox2d.getHeight(), reduce);
				b.bounding_verts_index0 = 1;
				
				rectf new_box;
				new_box.UpperLeftCorner.X = f->bbox2d.LowerRightCorner.Y;
				new_box.UpperLeftCorner.Y = f->bbox2d.UpperLeftCorner.X;
				new_box.LowerRightCorner.X = f->bbox2d.UpperLeftCorner.Y;
				new_box.LowerRightCorner.Y = f->bbox2d.LowerRightCorner.X;

				f->bbox2d = new_box;
				f->m_tangent = f->m_normal.crossProduct(f->m_tangent);
				
				//mapper.init(&geo_scene->get_total_geometry()->faces[f_i], 1,b.width,b.height);
				mapper.init(&geo_scene->get_total_geometry()->faces[f_i], 0,b.width,b.height);

				map_uvs(geo_scene, &geo_scene->edit_meshnode_interface, vector<int>{ f_i }, mapper, MAP_UVS_LIGHTMAP);

				b.faces = vector<int>{ f_i };

			} break;

			case SURFACE_GROUP_CYLINDER:
			{
				map_sphere_to_uv3 mapper;

				b.width = reduce_dimension_base2(sfg.radius * 2 * 3.1459 * 2, reduce);
				b.height = reduce_dimension_base2(sfg.height, reduce);
				b.faces = surface;


				//mapper.init(&sfg, pf, sfg_i,b.width,b.height);
				mapper.init(&sfg, pf, surface, sfg_i, b.width, b.height, true);

				for (int b_i : surface)
				{
					pf->calc_tangent(b_i);
					//cout << pf->faces[b_i].m_tangent.X << "," << pf->faces[b_i].m_tangent.Y << "," << pf->faces[b_i].m_tangent.Z << "\n";
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

				map_uvs(geo_scene, &geo_scene->edit_meshnode_interface, surface, mapper, MAP_UVS_LIGHTMAP);

				
				//std::cout << " width = " << b.width << "\n";
				//std::cout << " height = " << b.height << "\n";
				//std::cout << " trans = " << -mapper.m_min.X << " / " << -mapper.m_min.Y << "\n";
				//std::cout << " uv size = " << mapper.uv_width() << " / " << mapper.uv_height() << "\n";
				//std::cout << " uv off = " << mapper.m_min.X << " / " << mapper.m_min.Y << "\n";
				//std::cout << " scale = " << 1.0f / mapper.uv_width() << " / " << 1.0f / mapper.uv_height() << "\n";
				
				//m_scale.setScale(vector3df(1.0f / mapper.uv_width(), 1.0f / mapper.uv_height(), 1.0f));
				//m_translate.setTranslation(vector3df(-mapper.m_min.X,-mapper.m_min.Y,0.0f));

				//apply_transform_to_uvs(&geo_scene->edit_meshnode_interface, surface, MAP_UVS_LIGHTMAP, m_translate);
				//apply_transform_to_uvs(&geo_scene->edit_meshnode_interface, surface, MAP_UVS_LIGHTMAP, m_scale);

			} break;
			case SURFACE_GROUP_DOME:
			case SURFACE_GROUP_SPHERE:
			{

				map_sphere_to_uv3 mapper;

				b.width = reduce_dimension_base2(sfg.radius * 2 * 3.1459 * 2, reduce);
				b.height = b.width;
				b.faces = surface;

				mapper.init(&sfg, pf, surface, sfg_i, b.width, b.height, false);

				for (int b_i : surface)
				{
					pf->calc_tangent(b_i);
					//cout << pf->faces[b_i].m_tangent.X << "," << pf->faces[b_i].m_tangent.Y << "," << pf->faces[b_i].m_tangent.Z << "\n";
				
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

				map_uvs(geo_scene, &geo_scene->edit_meshnode_interface, surface, mapper, MAP_UVS_LIGHTMAP);

				//std::cout << " trans = " << -mapper.m_min.X << " / " << -mapper.m_min.Y << "\n";
				//std::cout << " scale = " << 1.0f / mapper.uv_width() << " / " << 1.0f / mapper.uv_height() << "\n";

				//m_scale.setScale(vector3df(1.0f / mapper.uv_width(), 1.0f / mapper.uv_height(), 1.0f));
				//m_translate.setTranslation(vector3df(-mapper.m_min.X, -mapper.m_min.Y, 0.0f));

				//apply_transform_to_uvs(&geo_scene->edit_meshnode_interface, surface, MAP_UVS_LIGHTMAP, m_translate);
				//apply_transform_to_uvs(&geo_scene->edit_meshnode_interface, surface, MAP_UVS_LIGHTMAP, m_scale);

			} break;
			/*
			case SURFACE_GROUP_DOME:
			case SURFACE_GROUP_SPHERE:
			{
				
				map_sphere_to_uv mapper;

				mapper.init(&sfg);

				map_uvs(geo_scene, &geo_scene->edit_meshnode_interface, surface, mapper, MAP_UVS_LIGHTMAP);

				b.faces = surface;
				b.width = reduce_dimension_base2(mapper.uv_width() * sfg.radius * 2 * 3.1459, reduce);
				b.height = reduce_dimension_base2(mapper.uv_height() * sfg.radius * 2 * 3.1459, reduce);

				//std::cout << " trans = " << -mapper.m_min.X << " / " << -mapper.m_min.Y << "\n";
				//std::cout << " scale = " << 1.0f / mapper.uv_width() << " / " << 1.0f / mapper.uv_height() << "\n";

				m_scale.setScale(vector3df(1.0f / mapper.uv_width(), 1.0f / mapper.uv_height(), 1.0f));
				m_translate.setTranslation(vector3df(-mapper.m_min.X, -mapper.m_min.Y, 0.0f));

				apply_transform_to_uvs(&geo_scene->edit_meshnode_interface, surface, MAP_UVS_LIGHTMAP, m_translate);
				apply_transform_to_uvs(&geo_scene->edit_meshnode_interface, surface, MAP_UVS_LIGHTMAP, m_scale);

			} break;
			*/
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

			for (int b_i : surface)
			{
				//pf->faces[b_i].m_tangent = pf->faces[b_i].m_tangent.crossProduct(pf->faces[b_i].m_normal);
				//pf->faces[b_i].m_tangent *= -1;
			}
		}

		*ret = b;
		++ret;
	}
}

void lightmaps_divideMaterialGroups(geometry_scene* geo_scene, std::vector<TextureMaterial>& material_groups)
{
	std::vector<TextureMaterial> ret;
	auto ret_back = std::back_inserter(ret);

	auto in_it = material_groups.begin();
	auto in_end = material_groups.end();

	polyfold* pf = geo_scene->get_total_geometry();

	for (int f_i = 0; f_i < pf->faces.size(); f_i++)
	{
		pf->faces[f_i].temp_b = false;
	}

	for (; in_it != in_end; ++in_it)
	{
		std::vector<lm_block> mg_blocks;
		auto mg_blocks_back = std::back_inserter(mg_blocks);

		for (int f_i : in_it->faces)
		{
			if (geo_scene->get_total_geometry()->faces[f_i].temp_b == true)
				continue;

			initialize_block(geo_scene, f_i, mg_blocks_back,1);
		}

		std::sort(mg_blocks.begin(), mg_blocks.end(),
			[&](lm_block& b_a, lm_block& b_b)
			{
				if (b_a.width == b_b.width)
				{
					return b_a.height > b_b.height;
				}
				else
					return b_a.width > b_b.width;
			});

		auto blocks_it = mg_blocks.begin();
		auto blocks_end = mg_blocks.end();

		while (!lightmaps_fill(&geo_scene->edit_meshnode_interface, blocks_it, blocks_end, ret_back, *in_it))
		{
			++ret_back;
		}

		++ret_back;
	}
	
	std::cout << "Created "<<ret.size() << " new material groups\n";

	material_groups = ret;
}

Lightmap_Manager::Lightmap_Manager()
{
	MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
	event_receiver = receiver;
}

void Lightmap_Manager::loadLightmapTextures(geometry_scene* geo_scene, const std::vector<TextureMaterial>& material_groups)
{
	SEvent event;
	event.EventType = EET_USER_EVENT;
	event.UserEvent.UserData1 = USER_EVENT_CLEAR_LIGHTMAP_TEXTURES;
	event_receiver->OnEvent(event);

	for (video::ITexture* tex: lightmap_textures)
	{
		device->getVideoDriver()->removeTexture(tex);
	}

	lightmap_textures.clear();

	for (int i = 0; i < material_groups.size(); i++)
	{
		std::stringstream ss;
		ss << "../projects/export/lightmap_" << i << ".bmp";
		video::ITexture* tex = device->getVideoDriver()->getTexture(ss.str().c_str());

		lightmap_textures.push_back(tex);

		for (int f_i : material_groups[i].faces)
		{
			MeshBuffer_Chunk chunk;
			chunk = geo_scene->final_meshnode_interface.get_mesh_buffer_by_face(f_i);
			chunk.buffer->getMaterial().setTexture(1, tex);

			chunk = geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(f_i);
			chunk.buffer->getMaterial().setTexture(1, tex);
		}
	}

	geo_scene->getMeshNode()->copyMaterials();

	event.EventType = EET_USER_EVENT;
	event.UserEvent.UserData1 = USER_EVENT_MATERIAL_GROUP_SELECTION_CHANGED;
	event_receiver->OnEvent(event);
}


