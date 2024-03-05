#include <irrlicht.h>
#include "LightMaps.h"
#include "csg_classes.h"
#include "utils.h"
#include "geometry_scene.h"
#include "tolerances.h"
#include <algorithm>

using namespace irr;
using namespace core;


u16 lightmaps_guessLMres(geometry_scene* geo_scene, int f_j)
{
	polyfold* pf = geo_scene->get_total_geometry();
	poly_face* f = &pf->faces[f_j];

	rect<f32> face_bbox;

	for (int i = 0; i < f->loops.size(); i++)
	{
		if (f->loops[i].vertices.size() > 0)
		{
			if (i == 0)
			{
				face_bbox.UpperLeftCorner.X = f->loops[i].min_x;
				face_bbox.UpperLeftCorner.Y = f->loops[i].min_z;
				face_bbox.LowerRightCorner.X = f->loops[i].max_x;
				face_bbox.LowerRightCorner.Y = f->loops[i].max_z;
			}
			else
			{
				face_bbox.addInternalPoint(vector2df(f->loops[i].min_x, f->loops[i].min_z));
				face_bbox.addInternalPoint(vector2df(f->loops[i].max_x, f->loops[i].max_z));
			}
		}
	}

	int square_len = std::max(face_bbox.getWidth(), face_bbox.getHeight());
	u16 ret = 0;

	for (int i = 0; i < 16; i++)
	{
		ret = 2 << i;
		if (ret > square_len)
			break;
	}

	return std::min(256, std::max(8, ret / 4));
}

struct texture_block
{
	int size = 128;
	int face = -1;
	int max_size = 512;

	rect<u16> coords;

	texture_block* quad[4] = { NULL,NULL,NULL,NULL };

	texture_block()
	{}

	texture_block(const texture_block& r)
	{
		size = r.size;
		face = r.face;
	}

	texture_block(int size_)
	{
		size = size_;
	}

	texture_block(int face_, int size_)
	{
		face = face_;
		size = size_;
	}

	~texture_block()
	{
		for (int i = 0; i < 4; i++)
			if (quad[i]) 
				delete quad[i];
	}

	bool get_my_block(int f_i, rect<u16> &block)
	{
		if (face == f_i)
		{
			block = coords;
			return true;
		}
		else if (quad[0] && quad[0]->get_my_block(f_i,block))
			return true;
		else if (quad[1] && quad[1]->get_my_block(f_i, block))
			return true;
		else if (quad[2] && quad[2]->get_my_block(f_i, block))
			return true;
		else if (quad[3] && quad[3]->get_my_block(f_i, block))
			return true;
		else 
			return false;
	}

	void dump(int tab, int n)
	{
		for (int i = 0; i < tab; i++)
			std::cout << " ";

		std::cout << n << ": ";

		std::cout << "face=" << face << ", ";
		std::cout << "size=" << size << ", ";
		std::cout << coords.UpperLeftCorner.X<<","<<coords.UpperLeftCorner.Y<<" / ";
		std::cout << coords.LowerRightCorner.X << "," << coords.LowerRightCorner.Y << "\n";

		for (int i = 0; i < 4; i++)
		{
			if (quad[i])
			{
				quad[i]->dump(tab + 1,i);
			}
		}
	}

	bool has_quads() {
		return quad[0] || quad[1] || quad[2] || quad[3];
	}

	bool has_empty_quads() {
		return (!quad[0]) || (!quad[1]) || (!quad[2]) || (!quad[3]);
	}

	int get_first_empty_quad()
	{
		for (int i = 0; i < 4; i++)
		{
			if (!quad[i])
				return i;
		}

		return -1;
	}

	void set_coords(vector2d<u16> origin)
	{
		coords = rect<u16>(origin, dimension2d<u16>(size -1, size -1));

		if(quad[0])
			quad[0]->set_coords(origin);
		if (quad[1])
			quad[1]->set_coords(origin + vector2d<u16>(size/2,0));
		if (quad[2])
			quad[2]->set_coords(origin + vector2d<u16>(0, size/2));
		if (quad[3])
			quad[3]->set_coords(origin + vector2d<u16>(size/2, size/2));
	}

	void upsize()
	{
		texture_block* r = new texture_block(*this);

		for (int i = 0; i < 4; i++)
			r->quad[i] = quad[i];

		if (has_quads() || face != -1)
			quad[0] = r;
		else
			quad[0] = NULL;
		quad[1] = NULL;
		quad[2] = NULL;
		quad[3] = NULL;

		size *= 2;
		face = -1;
	}

	bool try_add(const texture_block& r)
	{
		if (r.size == size)
		{
			if (face == -1 && !has_quads())
			{
				face = r.face;
				return true;
			}
			else return false;
		}
		else if (r.size == size / 2)
		{
			if (face == -1 && has_empty_quads())
			{
				int j = get_first_empty_quad();
				quad[j] = new texture_block(r);

				return true;
			}
			else return false;
		}
		else if (r.size < size / 2)
		{
			if(face == -1)
				for (int i = 0; i < 4; i++)
				{
					if (quad[i] && quad[i]->try_add(r))
						return true;
					else if (!quad[i])
					{
						quad[i] = new texture_block( size / 2);
						return quad[i]->try_add(r);
					}
				}
			return false;
		}
		return false;
	}

	bool add(const texture_block& r)
	{
		while (!try_add(r))
		{
			if (size < max_size)
			{
				this->upsize();
			}
			else
				return false;
		}
		return true;
	}
};

template<typename out_Type>
bool lightmaps_fill(geometry_scene* geo_scene, std::vector<int>::iterator& faces_it, std::vector<int>::iterator faces_end, out_Type out, const TextureMaterial& copy_from)
{
	texture_block new_block(128);
	texture_block face_block(128);

	TextureMaterial ret;

	ret.materialGroup = copy_from.materialGroup;
	ret.texture = copy_from.texture;

	auto ret_blocks_back = std::back_inserter(ret.blocks);
	auto ret_faces_back = std::back_inserter(ret.faces);

	rect<u16> my_block;
	
	for (; faces_it != faces_end; ++faces_it)
	{
		int f_i = *faces_it;

		face_block.face = f_i;
		face_block.size = geo_scene->get_total_geometry()->faces[f_i].lightmap_res;

		if (!new_block.add(face_block))
		{
			*out = ret;
			return false;
		}
		
		new_block.set_coords(vector2d<u16>(0, 0));
		new_block.get_my_block(f_i, my_block);

		*ret_faces_back = f_i;
		*ret_blocks_back = my_block;

		++ret_faces_back;
		++ret_blocks_back;

		ret.lightmap_size = new_block.size;
	}

	*out = ret;

	return true;
}

void lightmaps_divideMaterialGroups(geometry_scene* geo_scene, std::vector<TextureMaterial>& material_groups)
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
				return	geo_scene->get_total_geometry()->faces[f_a].lightmap_res >
						geo_scene->get_total_geometry()->faces[f_b].lightmap_res;
			});

		while (!lightmaps_fill(geo_scene, faces_it, faces_end, ret_back, *in_it))
		{
			++ret_back;
		}

		++ret_back;
	}

	//std::cout << ret.size() << " new material groups\n";

	material_groups = ret;
}


