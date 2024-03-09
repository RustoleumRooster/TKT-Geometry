#include <irrlicht.h>
#include "LightMaps.h"
#include "csg_classes.h"
#include "utils.h"
#include "geometry_scene.h"
#include "tolerances.h"
#include <algorithm>
#include <math.h>

using namespace irr;
using namespace core;

void guess_lightmaps_dimension(geometry_scene* geo_scene, int f_j)
{
	polyfold* pf = geo_scene->get_total_geometry();
	poly_face* f = &pf->faces[f_j];

	std::cout << f_j << " " << f->bbox2d.getWidth() << "," << f->bbox2d.getHeight() << "\n";

	double log_width = log2(f->bbox2d.getWidth());
	u16 lwi = static_cast<u16>(floor(log_width));
	lwi -= 3;
	lwi = lwi > 2 ? lwi : 2;

	double log_height = log2(f->bbox2d.getHeight());
	u16 lhi = static_cast<u16>(floor(log_height));
	lhi -= 3;
	lhi = lhi > 2 ? lhi : 2;

	f->lightmap_dim = dimension2du(exp2(lwi), exp2(lhi));
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


template<typename out_Type>
bool lightmaps_fill2(geometry_scene* geo_scene, std::vector<int>::iterator& faces_it, std::vector<int>::iterator faces_end, out_Type out, const TextureMaterial& copy_from)
{
	texture_block2 new_block(128);
	texture_block2 face_block(128);

	TextureMaterial ret;

	ret.materialGroup = copy_from.materialGroup;
	ret.texture = copy_from.texture;

	auto ret_blocks_back = std::back_inserter(ret.blocks);
	auto ret_faces_back = std::back_inserter(ret.faces);

	rect<u16> my_block;

	std::vector<int> face_stack;

	for (; faces_it != faces_end; ++faces_it)
	{
		int f_i = *faces_it;

		face_block.dimension = geo_scene->get_total_geometry()->faces[f_i].lightmap_dim;

		if (!new_block.try_add(face_block))
		{
			*out = ret;
			return false;
		}

		*ret_faces_back = f_i;
		*ret_blocks_back = face_block.coords;

		++ret_faces_back;
		++ret_blocks_back;

		ret.lightmap_size = new_block.dimension.Width;
	}

	*out = ret;

	return true;
}


void lightmaps_divideMaterialGroups2(geometry_scene* geo_scene, std::vector<TextureMaterial>& material_groups)
{
	std::vector<TextureMaterial> ret;
	auto ret_back = std::back_inserter(ret);

	auto in_it = material_groups.begin();
	auto in_end = material_groups.end();

	for (; in_it != in_end; ++in_it)
	{
		auto faces_it = in_it->faces.begin();
		auto faces_end = in_it->faces.end();

		std::sort(in_it->faces.begin(), in_it->faces.end(),
			[&](int f_a, int f_b)
			{
				const poly_face& fa = geo_scene->get_total_geometry()->faces[f_a];
				const poly_face& fb = geo_scene->get_total_geometry()->faces[f_b];

				if (fa.lightmap_dim.Width == fb.lightmap_dim.Width)
				{
					return fa.lightmap_dim.Height > fb.lightmap_dim.Height;
				}
				else 
					return	fa.lightmap_dim.Width > fb.lightmap_dim.Width;
			});
		
		while (!lightmaps_fill2(geo_scene, faces_it, faces_end, ret_back, *in_it))
		{
			++ret_back;
		}

		++ret_back;
	}
	
	std::cout << "Created "<<ret.size() << " new material groups\n";

	material_groups = ret;
}


