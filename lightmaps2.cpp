#include <irrlicht.h>
#include "LightMaps.h"
#include "csg_classes.h"
#include "utils.h"
#include "geometry_scene.h"
#include "tolerances.h"
#include <algorithm>
#include <math.h>
#include "BufferManager.h"

using namespace irr;
using namespace core;

class map_cylinder_to_uv
{
public:

	vector3df iY,v0,v1,r0;
	f32 m_height, m_radius;

	f32 height() { return m_height; }
	f32 width() { return m_radius * 2 * 3.14; }

	void init(const surface_group* sg)
	{
		r0 = sg->point;
		v0 = sg->vec;
		v1 = sg->vec1;
		m_height = sg->height;
		m_radius = sg->radius;

		iY = v0.crossProduct(v1);
		iY.normalize();
	}

	vector2df calc(const vector3df& face_normal, const vector3df& pos)
	{
		vector2df k;
		vector3df r = pos - r0;

		k.X = r.dotProduct(v1);
		k.Y = r.dotProduct(iY);

		f32 theta = k.getAngleTrig();

		if (fabs(theta - 360.0) < 0.001 || fabs(theta) < 0.001)
		{
			if (face_normal.dotProduct(iY) * r.dotProduct(face_normal) > 0)
				theta = 0.0;
			else
			{
				theta = 360.0;
			}
		}

		vector3df uv(theta / 90, pos.dotProduct(v0) / 128, 0);

		return vector2df(-uv.X, uv.Y);
	}
};

class map_sphere_to_uv
{
	surface_group* sfg = NULL;

public:
	map_sphere_to_uv() {}
	f32 width() { return 16; }
	f32 height() { return 16; }

	void init(surface_group* sg)
	{
		sfg = sg;
	}

	vector2df calc(const vector3df& face_normal, const vector3df& pos)
	{
		core::vector3df r = pos - sfg->point;
		r.normalize();
		core::vector3df rr = sfg->vec.crossProduct(r);
		rr.normalize();

		f32 alpha = acos(r.dotProduct(sfg->vec));
		f32 theta = acos(rr.dotProduct(sfg->vec1));
		/*
				if(fabs(alpha)<0.001 || fabs(alpha-pi)<0.001)
					{
						core::vector3df centerv(0,0,0);
						for(int j=0;j<t_h.vertices.size();j++)
						{
							centerv += t_h.vertices[j];
						}
						centerv /= t_h.vertices.size();

						r = centerv-t_h.surface_info.point;
						r.normalize();
						rr = t_h.surface_info.vec.crossProduct(r);
						rr.normalize();
						theta = acos(rr.dotProduct(t_h.surface_info.vec1));
					}
		*/
		return vector2df(theta * 4.0 / 3.1459, alpha * 4.0 / 3.1459);
	}
};

class map_dome_to_uv
{
	surface_group* sfg;
	vector3df vec3;

public:

	f32 width() { return 16; }
	f32 height() { return 16; }

	void init(surface_group* sg)
	{
		sfg = sg;
		vec3 = sfg->vec.crossProduct(sfg->vec1);
		vec3.normalize();
	}

	vector2df calc(const vector3df& face_normal, const vector3df& pos)
	{
		core::vector3df r = pos - sfg->point;
		r.normalize();
		core::vector3df rr = sfg->vec.crossProduct(r);
		rr.normalize();

		f32 alpha = acos(r.dotProduct(sfg->vec));
		f32 theta = rr.dotProduct(sfg->vec1);
		f32 theta1 = rr.dotProduct(vec3);
		return vector2df(alpha * theta, alpha * theta1);
	}
};

f32 reduce_dimension_base2(f32 dim, int n = 1)
{
	double log_width = log2(dim);
	u16 lwi = static_cast<u16>(floor(log_width));
	lwi -= n;
	lwi = lwi > n ? lwi : n;

	return exp2(lwi);
}

void guess_lightmaps_dimension(geometry_scene* geo_scene, int f_j)
{
	polyfold* pf = geo_scene->get_total_geometry();
	poly_face* f = &pf->faces[f_j];

	f->lightmap_dim = dimension2du(reduce_dimension_base2(f->bbox2d.getWidth(),2), 
		reduce_dimension_base2(f->bbox2d.getHeight(),2));
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

struct lm_block
{
	int face;
	int v0_index;
	u32 width;
	u32 height;
	f32 offset_x;
	f32 offset_y;
	f32 uv_width;
	f32 uv_height;
	std::vector<lm_block> subblocks;
};

struct extent_2D
{
	int n = 0;
	f32 min_x;
	f32 min_y;
	f32 max_x;
	f32 max_y;

	void extend(f32 x, f32 y)
	{
		if (n == 0)
		{
			min_x = max_x = x;
			min_y = max_y = y;
		}
		else
		{
			min_x = fmin(min_x, x);
			max_x = fmax(max_x, x);
			min_y = fmin(min_y, y);
			max_y = fmax(max_y, y);
		}
		n++;
	}
	void extend(vector2df v)
	{
		extend(v.X, v.Y);
	}

	f32 width() const { return max_x - min_x; }
	f32 height() const { return max_y - min_y; }
};


template<typename out_Type>
bool lightmaps_fill3(geometry_scene* geo_scene, std::vector<lm_block>::iterator& blocks_it, std::vector<lm_block>::iterator blocks_end, out_Type out, const TextureMaterial& copy_from)
{
	texture_block2 new_block(128);
	texture_block2 face_block(128);

	TextureMaterial ret;

	ret.materialGroup = copy_from.materialGroup;
	ret.texture = copy_from.texture;

	auto ret_records_back = std::back_inserter(ret.records);

	rect<u16> my_block;

	for (; blocks_it != blocks_end; ++blocks_it)
	{
		face_block.dimension = dimension2du(blocks_it->width, blocks_it->height);

		if (!new_block.try_add(face_block))
		{
			*out = ret;
			return false;
		}

		if (blocks_it->face != -1)
		{
			TextureMaterial::lightmap_record rec;

			rec.face = blocks_it->face;
			rec.block = face_block.coords;

			rec.block.UpperLeftCorner.X += 1;
			rec.block.UpperLeftCorner.Y += 1;
			rec.block.LowerRightCorner.X -= 1;
			rec.block.LowerRightCorner.Y -= 1;

			rec.face_v0_index = blocks_it->v0_index;
			*ret_records_back = rec;
			++ret_records_back;
		}
		else
		{
			for (int i = 0; i < blocks_it->subblocks.size(); i++)
			{
				TextureMaterial::lightmap_record rec;

				vector2d<u16> UL = face_block.coords.UpperLeftCorner;
				vector2d<u16> LR = face_block.coords.UpperLeftCorner;

				lm_block& b = blocks_it->subblocks[i];

				UL.X += round32(1.0 + ((b.offset_x / blocks_it->uv_width) * f32(blocks_it->width - 2)));
				UL.Y += round32(1.0 + ((b.offset_y / blocks_it->uv_height) * f32(blocks_it->height - 2)));

				LR.X += round32(1.0 + (((b.offset_x + b.uv_width ) / f32(blocks_it->uv_width))
						* f32(blocks_it->width - 2)));

				LR.Y += round32(1.0 + (((b.offset_y + b.uv_height) / f32(blocks_it->uv_height))
						* f32(blocks_it->height - 2)));

				rec.face = b.face;
				rec.block = rect<u16>(UL, LR);
				rec.face_v0_index = b.v0_index;

				*ret_records_back = rec;
				++ret_records_back;
			}
		}

		ret.lightmap_size = new_block.dimension.Width;
	}

	*out = ret;

	return true;
}

template<typename back_type, typename map_type>
void initialize_special_block(geometry_scene* geo_scene, int f_i, back_type ret, map_type& mapper)
{
	std::vector<int> surface = geo_scene->getSurfaceFromFace(f_i);
	lm_block b;
	b.face = -1;
	polyfold* pf = geo_scene->get_total_geometry();

	surface_group* sfg = pf->getFaceSurfaceGroup(f_i);
	mapper.init(sfg);

	b.width = reduce_dimension_base2(mapper.width(),2);
	b.height = reduce_dimension_base2(mapper.height(),2);

	extent_2D ex;

	std::cout << "h=" << b.height << ", w=" << b.width << "\n";

	for (int b_i : surface)
	{
		lm_block sb;
		extent_2D sex;

		if (pf->faces[b_i].temp_b == false)
		{
			sb.face = b_i;
			vector3df points[4];

			pf->faces[b_i].get3DBoundingQuad(points);

			vector2df v[4];
			for (int i = 0; i < 4; i++)
			{
				v[i] = mapper.calc(pf->faces[b_i].m_normal, points[i]);
				sex.extend(v[i]);
				ex.extend(v[i]);
			}

			sb.v0_index = 0;

			int rect_target_corner;

			if (b.width > b.height)
			{
				rect_target_corner = 0;
			}
			else
				rect_target_corner = 1;


			for (int i = 0; i < 3; i++)
			{
				vector2df v0 = mapper.calc(pf->faces[b_i].m_normal, points[rect_target_corner]);

				if (fabs(v0.X - sex.min_x) < 0.00001 && fabs(v0.Y - sex.min_y) < 0.00001)
					break;

				vector3df new_points[4];

				new_points[0] = points[3];
				new_points[1] = points[0];
				new_points[2] = points[1];
				new_points[3] = points[2];


				sb.v0_index +=1;
				memcpy(points, new_points, sizeof(vector3df) * 4);
			}

			sb.face = b_i;
			sb.uv_width = sex.width();
			sb.uv_height = sex.height();

			sb.offset_x = sex.min_x;
			sb.offset_y = sex.min_y;

			b.subblocks.push_back(sb);

			pf->faces[b_i].temp_b = true;
		}
	}

	if(b.width > b.height)
	{
		b.uv_width = ex.width();
		b.uv_height = ex.height();

		for (lm_block& sb : b.subblocks)
		{
			sb.offset_x -= ex.min_x;
			sb.offset_y -= ex.min_y;
		}
	}
	else
	{
		b.uv_width = ex.height();
		b.uv_height = ex.width();

		for (lm_block& sb : b.subblocks)
		{
			f32 t = sb.uv_height;
			sb.uv_height = sb.uv_width;
			sb.uv_width = t;

			t = sb.offset_x;
			sb.offset_x = sb.offset_y;
			sb.offset_y = t;
		}

		u32 t = b.width;
		b.width = b.height;
		b.height = t;

		for (lm_block& sb : b.subblocks)
		{
			sb.offset_x -= ex.min_y;
			sb.offset_y -= ex.min_x;
		}
	}

	std::cout << "width: " << b.uv_width << " height: " << b.uv_height << "\n";

	*ret = b;
	++ret;
}

template<typename back_type>
void initialize_block(geometry_scene* geo_scene, int f_i, back_type ret)
{
	polyfold* pf = geo_scene->get_total_geometry();

	if (pf->faces[f_i].loops.size() > 0 && pf->faces[f_i].temp_b == false)
	{
		surface_group& sfg = *pf->getFaceSurfaceGroup(f_i);

		switch (sfg.type)
		{
		case SURFACE_GROUP_STANDARD:
		case SURFACE_GROUP_CUSTOM_UVS_BRUSH:
		case SURFACE_GROUP_CUSTOM_UVS_GEOMETRY:
		{
			if (pf->faces[f_i].temp_b == false)
			{
				lm_block b;

				b.face = f_i;
				b.width = pf->faces[f_i].lightmap_dim.Width;
				b.height = pf->faces[f_i].lightmap_dim.Height;
				b.v0_index = 1;

				*ret = b;
				++ret;

				pf->faces[f_i].temp_b = true;
			}
		} break;
		case SURFACE_GROUP_CYLINDER:
		{
			
			map_cylinder_to_uv mapper;
			initialize_special_block(geo_scene, f_i, ret, mapper);

		} break;
		case SURFACE_GROUP_DOME:
		{
			map_dome_to_uv mapper;
			initialize_special_block(geo_scene, f_i, ret, mapper);

		} break;
		case SURFACE_GROUP_SPHERE:
		{
			map_sphere_to_uv mapper;
			initialize_special_block(geo_scene, f_i, ret, mapper);

		} break;
		default:
			std::cout << "unknown surface properties\n";
			break;
		}
	}
}

void lightmaps_divideMaterialGroups2(geometry_scene* geo_scene, std::vector<TextureMaterial>& material_groups)
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

		while (!lightmaps_fill3(geo_scene, blocks_it, blocks_end, ret_back, *in_it))
		{
			++ret_back;
		}

		++ret_back;
	}
	
	std::cout << "Created "<<ret.size() << " new material groups\n";

	material_groups = ret;
}


