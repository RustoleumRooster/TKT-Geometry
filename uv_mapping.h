#ifndef _UV_MAPPING_H_
#define _UV_MAPPING_H_

#include <irrlicht.h>
#include "csg_classes.h"
#include "BufferManager.h"
#include "geometry_scene.h"

using namespace irr;
using namespace core;

class map_face_to_uvs
{
public:
	vector3df verts[4];
	vector3df u_vec;
	vector3df v_vec;

	f32 max_height() { return vector3df(verts[3] - verts[3]).getLength(); }
	f32 max_width() { return vector3df(verts[1] - verts[0]).getLength(); }

	void init(const poly_face* f, int v0_idx)
	{
		//int v0_idx = materials_used[m_i].records[i].face_v0_index;
		f->get3DBoundingQuad(verts, v0_idx);

		u_vec = verts[1] - verts[0];
		v_vec = verts[3] - verts[0];

		u_vec.normalize();
		v_vec.normalize();

		u_vec.X = fabs(u_vec.X) < 0.0001 ? 0 : u_vec.X;
		u_vec.Y = fabs(u_vec.Y) < 0.0001 ? 0 : u_vec.Y;
		v_vec.X = fabs(v_vec.X) < 0.0001 ? 0 : v_vec.X;
		v_vec.Y = fabs(v_vec.Y) < 0.0001 ? 0 : v_vec.Y;
	}

	vector2df calc(const vector3df& face_normal, const vector3df& pos) const
	{
		float_t u = vector3df(pos - verts[0]).dotProduct(u_vec) / vector3df(verts[1] - verts[0]).getLength();
		float_t v = vector3df(pos - verts[0]).dotProduct(v_vec) / vector3df(verts[3] - verts[0]).getLength();

		return vector2df(u, v);
	}

};

class map_cylinder_to_uv
{

public:

	vector3df iY, v0, v1, r0;
	f32 m_height, m_radius;

	f32 max_height() { return m_height; }
	f32 max_width() { return m_radius * 2 * 3.14; }

	vector2df m_min, m_max;
	int n = 0;

	f32 uv_width() { return m_max.X - m_min.X; }
	f32 uv_height() { return m_max.Y - m_min.Y; }

	void init(const surface_group* sg)
	{
		r0 = sg->point;
		v0 = sg->vec;
		v1 = sg->vec1;
		m_height = sg->height;
		m_radius = sg->radius;

		iY = v0.crossProduct(v1);
		iY.normalize();
		n = 0;
	}

	vector2df calc(const vector3df& face_normal, const vector3df& pos)
	{
		vector2df k;
		vector3df r = pos - r0;

		k.X = r.dotProduct(v1);
		k.Y = r.dotProduct(iY);

		f32 theta = k.getAngleTrig();

		if (fabs(theta - 360.0f) < 0.001f || fabs(theta) < 0.001f)
		{
			if (face_normal.dotProduct(iY) * r.dotProduct(face_normal) > 0)
				theta = 0.0f;
			else
			{
				theta = 360.0f;
			}
		}

		//vector3df uv(theta / 90, pos.dotProduct(v0) / 128, 0);
		vector2df uv(theta / 360.0f, 0.5f + (r.dotProduct(v0) / m_height));

		m_min = n > 0 ? vector2df(fmin(m_min.X, uv.X), fmin(m_min.Y, uv.Y)) : uv;
		m_max = n > 0 ? vector2df(fmax(m_max.X, uv.X), fmax(m_max.Y, uv.Y)) : uv;

		n++;

		return uv;
	}
};

class map_sphere_to_uv
{
	surface_group* sfg = NULL;
	f32 m_radius;

public:
	map_sphere_to_uv() {}

	f32 pi = 3.141592653;

	f32 max_width() { return m_radius * 2 * 3.14; }
	f32 max_height() { return m_radius * 2 * 3.14; }

	vector2df m_min, m_max;
	int n = 0;

	f32 uv_width() { return m_max.X - m_min.X; }
	f32 uv_height() { return m_max.Y - m_min.Y; }

	void init(surface_group* sg)
	{
		sfg = sg;
		m_radius = sg->radius;
	}

	vector2df calc(const vector3df& face_normal, const vector3df& pos)
	{
		core::vector3df r = pos - sfg->point;
		r.normalize();
		core::vector3df rr = sfg->vec.crossProduct(r);
		rr.normalize();

		f32 alpha = acos(r.dotProduct(sfg->vec));
		//f32 az = acos(r.dotProduct(sfg->vec1));

		vector3df iY = sfg->vec.crossProduct(sfg->vec1);
		iY.normalize();

		vector2df k;

		k.X = r.dotProduct(iY);
		k.Y = r.dotProduct(sfg->vec1);

		f32 az = k.getAngleTrig();

		if (fabs(az - 360.0f) < 0.001f || fabs(az) < 0.001f)
		{
			if (face_normal.dotProduct(sfg->vec1) * r.dotProduct(face_normal) > 0)
				az = 0.0f;
			else
			{
				az = 360.0f;
			}
		}
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
			}*/
		
		vector2df uv = vector2df(az / 360.0f, alpha / 3.1459);

		m_min = n > 0 ? vector2df(fmin(m_min.X, uv.X), fmin(m_min.Y, uv.Y)) : uv;
		m_max = n > 0 ? vector2df(fmax(m_max.X, uv.X), fmax(m_max.Y, uv.Y)) : uv;

		n++;
		
		return uv;
	}
};

class map_dome_to_uv
{
	surface_group* sfg;
	vector3df vec3;

public:

	f32 max_width() { return 16; }
	f32 max_height() { return 16; }

	f32 u_min() { return 0; }
	f32 u_max() { return 360; }
	f32 v_min() { return -0.5; }
	f32 v_max() { return  0.5; }

	void init(surface_group* sg)
	{
		sfg = sg;
		vec3 = sfg->vec.crossProduct(sfg->vec1);
		vec3.normalize();
	}

	vector2df calc(const vector3df& face_normal, const vector3df& pos) const
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

enum {
	MAP_UVS_TEXTURE = 0,
	MAP_UVS_LIGHTMAP
};

template<class map_type>
void map_uvs(geometry_scene* geo_scene, MeshNode_Interface* mesh_node, const std::vector<int>& surface, map_type& mapper, int uv_type)
{
	polyfold* pf = geo_scene->get_total_geometry();

	video::S3DVertex2TCoords* vtx;

	for (int b_i : surface)
	{
		MeshBuffer_Chunk chunk = mesh_node->get_mesh_buffer_by_face(b_i);
		std::cout << b_i << ":\n";
		if (chunk.buffer)
		{
			for (int i = chunk.begin_i; i < chunk.end_i; i++)
			{
				u16 idx = chunk.buffer->getIndices()[i];
				vtx = &((video::S3DVertex2TCoords*)chunk.buffer->getVertices())[idx];

				vector2df uv = mapper.calc(pf->faces[b_i].m_normal, vtx->Pos);

				if (uv_type == MAP_UVS_TEXTURE)
				{
					vtx->TCoords.set(uv.X, uv.Y);
				}
				else if (uv_type == MAP_UVS_LIGHTMAP)
				{
					vtx->TCoords2.set(uv.X, uv.Y);
					std::cout << uv.X << "," << uv.Y << "\n";
				}
			}
		}
	}
}

void apply_transform_to_uvs(MeshNode_Interface* mesh_node, const std::vector<int>& surface, int uv_type, matrix4 mat);

#endif