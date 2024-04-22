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

using namespace irr;
using namespace core;
using namespace std;

f32 reduce_dimension_base2(f32 dim, int n = 1)
{
	double log_width = log2(dim);
	u16 lwi = static_cast<u16>(floor(log_width));
	lwi -= n;
	lwi = lwi > 3 ? lwi : 3;

	return exp2(lwi);
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
	u32 width;
	u32 height;
	bool bFlipped = false;
};

template<typename out_Type>
bool lightmaps_fill(geometry_scene* geo_scene, std::vector<lm_block>::iterator& blocks_it, std::vector<lm_block>::iterator blocks_end, out_Type out, const TextureMaterial& copy_from)
{
	texture_block2 new_block(256);
	texture_block2 face_block(128);

	TextureMaterial ret;

	ret.materialGroup = copy_from.materialGroup;
	ret.texture = copy_from.texture;

	auto ret_records_back = std::back_inserter(ret.records);

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
	
		matrix4 m;

		if (blocks_it->bFlipped)
		{
			m.setScale(vector3df(	(f32)(blocks_it->width - 3) / (f32)new_block.dimension.Width, 
									(f32)(blocks_it->height - 3) / (f32)new_block.dimension.Height, 1.0f));

			m.setTranslation(vector3df(	(f32)(face_block.coords.UpperLeftCorner.X + 1.5f) / (f32)new_block.dimension.Width, 
										(f32)(face_block.coords.UpperLeftCorner.Y + 1.5f) / (f32)new_block.dimension.Height, 0));

			apply_transform_to_uvs(&geo_scene->edit_meshnode_interface, blocks_it->faces, MAP_UVS_LIGHTMAP, flip_mat);
			apply_transform_to_uvs(&geo_scene->edit_meshnode_interface, blocks_it->faces, MAP_UVS_LIGHTMAP, m);
		}
		else
		{
			m.setScale(vector3df(	(f32)(blocks_it->width - 3) / (f32)new_block.dimension.Width,
									(f32)(blocks_it->height - 3) / (f32)new_block.dimension.Height, 1));

			m.setTranslation(vector3df(	(f32)(face_block.coords.UpperLeftCorner.X + 1.5f) / (f32)new_block.dimension.Width,
										(f32)(face_block.coords.UpperLeftCorner.Y + 1.5f) / (f32)new_block.dimension.Height, 0));

			apply_transform_to_uvs(&geo_scene->edit_meshnode_interface, blocks_it->faces, MAP_UVS_LIGHTMAP, m);
		}
			
		for (int f_j : blocks_it->faces)
		{
			TextureMaterial::lightmap_record rec;

			rec.face = f_j;

			*ret_records_back = rec;
			++ret_records_back;
		}
		
		ret.lightmap_size = new_block.dimension.Width;
	}

	*out = ret;

	return true;
}

template<typename back_type>
void initialize_block(geometry_scene* geo_scene, int f_i, back_type ret)
{
	polyfold* pf = geo_scene->get_total_geometry();

	if (pf->faces[f_i].loops.size() > 0 && pf->faces[f_i].temp_b == false)
	{
		lm_block b;

		vector<int> surface;
		surface_group& sfg = *pf->getFaceSurfaceGroup(f_i);

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
				map_face_to_uvs mapper;

				pf->calc_tangent(f_i);
				mapper.init(&geo_scene->get_total_geometry()->faces[f_i], 1);

				map_uvs(geo_scene, &geo_scene->edit_meshnode_interface, vector<int>{ f_i }, mapper, MAP_UVS_LIGHTMAP);

				b.faces = vector<int>{ f_i };

				poly_face* f = &pf->faces[f_i];

				b.width = reduce_dimension_base2(f->bbox2d.getWidth(), 1);
				b.height = reduce_dimension_base2(f->bbox2d.getHeight(), 1);
				
			} break;

			case SURFACE_GROUP_CYLINDER:
			{
				map_cylinder_to_uv mapper;
				mapper.init(&sfg);

				map_uvs(geo_scene, &geo_scene->edit_meshnode_interface, surface, mapper, MAP_UVS_LIGHTMAP);

				b.faces = surface;
				b.width = reduce_dimension_base2(mapper.uv_width() * sfg.radius * 2 * 3.1459, 1);
				b.height = reduce_dimension_base2(mapper.uv_height() * sfg.height, 1);

				std::cout << b.width << "x" << b.height << " cylinder\n";
			} break;

			case SURFACE_GROUP_DOME:
			case SURFACE_GROUP_SPHERE:
			{
				map_sphere_to_uv mapper;

				mapper.init(&sfg);

				map_uvs(geo_scene, &geo_scene->edit_meshnode_interface, surface, mapper, MAP_UVS_LIGHTMAP);

				b.faces = surface;
				b.width = reduce_dimension_base2(mapper.uv_width() * sfg.radius * 2 * 3.1459, 1);
				b.height = reduce_dimension_base2(mapper.uv_height() * sfg.radius * 2 * 3.1459, 1);

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

			initialize_block(geo_scene, f_i, mg_blocks_back);
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

		while (!lightmaps_fill(geo_scene, blocks_it, blocks_end, ret_back, *in_it))
		{
			++ret_back;
		}

		++ret_back;
	}
	
	std::cout << "Created "<<ret.size() << " new material groups\n";

	material_groups = ret;
}


