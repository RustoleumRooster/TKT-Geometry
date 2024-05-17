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

	void calc(video::S3DVertex2TCoords** V) const
	{
		V[0]->TCoords2.X = vector3df(V[0]->Pos - verts[0]).dotProduct(u_vec) / vector3df(verts[1] - verts[0]).getLength();
		V[0]->TCoords2.Y = vector3df(V[0]->Pos - verts[0]).dotProduct(v_vec) / vector3df(verts[3] - verts[0]).getLength();

		V[1]->TCoords2.X = vector3df(V[1]->Pos - verts[0]).dotProduct(u_vec) / vector3df(verts[1] - verts[0]).getLength();
		V[1]->TCoords2.Y = vector3df(V[1]->Pos - verts[0]).dotProduct(v_vec) / vector3df(verts[3] - verts[0]).getLength();

		V[2]->TCoords2.X = vector3df(V[2]->Pos - verts[0]).dotProduct(u_vec) / vector3df(verts[1] - verts[0]).getLength();
		V[2]->TCoords2.Y = vector3df(V[2]->Pos - verts[0]).dotProduct(v_vec) / vector3df(verts[3] - verts[0]).getLength();

	}

};

class map_cylinder_to_uv
{

public:

	vector3df iY, v0, v1, r0;
	f32 m_height, m_radius;

	//f32 height() { return m_height; }
	//f32 width() { return m_radius * 2 * 3.14; }

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

	void calc(video::S3DVertex2TCoords** V)
	{
		vector3df face_normal = vector3df(V[2]->Pos - V[0]->Pos).crossProduct(vector3df(V[1]->Pos - V[0]->Pos));

		for (int i = 0; i < 3; i++)
		{
			vector2df k;
			vector3df r = V[i]->Pos - r0;

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

			V[i]->TCoords2 = uv;

			m_min = n > 0 ? vector2df(fmin(m_min.X, uv.X), fmin(m_min.Y, uv.Y)) : uv;
			m_max = n > 0 ? vector2df(fmax(m_max.X, uv.X), fmax(m_max.Y, uv.Y)) : uv;

			n++;
		}
	}
};

class map_sphere_to_uv
{
	surface_group* sfg = NULL;
	f32 m_radius;
	vector3df iY;

public:
	map_sphere_to_uv() {}

	f32 pi = 3.141592653f;

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
		iY = sfg->vec.crossProduct(sfg->vec1);
		iY.normalize();
	}

	void calc(video::S3DVertex2TCoords** V)
	{
		vector3df face_normal = vector3df(V[1]->Pos - V[0]->Pos).crossProduct(vector3df(V[2]->Pos - V[1]->Pos));

		int recalc = -1;

		for (int i = 0; i < 3; i++)
		{
			core::vector3df r = V[i]->Pos - sfg->point;
			r.normalize();
			core::vector3df rr = sfg->vec.crossProduct(r);
			rr.normalize();

			f32 alpha = acos(r.dotProduct(sfg->vec));

			vector2df k;

			k.X = r.dotProduct(sfg->vec1);
			k.Y = r.dotProduct(iY);

			f32 az = k.getAngleTrig();

			if (fabs(az - 360.0f) < 0.001f || fabs(az) < 0.001f)
			{
				if (face_normal.dotProduct(iY) * r.dotProduct(face_normal) > 0)
				{
					//std::cout << "1\n";
					az = 0.0f;
				}
				else
				{
					//std::cout << "0\n";
					az = 360.0f;
				}
			}

			if (fabs(fabs(r.dotProduct(sfg->vec)) - 1.0f) < 0.0001f)
			{
				recalc = i;
			}

			vector2df uv = vector2df(az / 360.0f, alpha / 3.1459f);

			V[i]->TCoords2 = uv;

			m_min = n > 0 ? vector2df(fmin(m_min.X, uv.X), fmin(m_min.Y, uv.Y)) : uv;
			m_max = n > 0 ? vector2df(fmax(m_max.X, uv.X), fmax(m_max.Y, uv.Y)) : uv;

			n++;
		}

		if (recalc != -1)
		{
			f32 left = fmin(V[(recalc + 1) % 3]->TCoords2.X, V[(recalc + 2) % 3]->TCoords2.X);
			f32 right = fmax(V[(recalc + 1) % 3]->TCoords2.X, V[(recalc + 2) % 3]->TCoords2.X);

			V[recalc]->TCoords2.X = left + (right - left) * 0.5f;
		}
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

	video::S3DVertex2TCoords* vtx[3];

	for (int b_i : surface)
	{
		MeshBuffer_Chunk chunk = mesh_node->get_mesh_buffer_by_face(b_i);
		//std::cout << b_i << ":\n";
		if (chunk.buffer)
		{
			for (int i = chunk.begin_i; i < chunk.end_i; i+=3)
			{
				u16 idx0 = chunk.buffer->getIndices()[i];
				u16 idx1 = chunk.buffer->getIndices()[i+1];
				u16 idx2 = chunk.buffer->getIndices()[i+2];

				vtx[0] = &((video::S3DVertex2TCoords*)chunk.buffer->getVertices())[idx0];
				vtx[1] = &((video::S3DVertex2TCoords*)chunk.buffer->getVertices())[idx1];
				vtx[2] = &((video::S3DVertex2TCoords*)chunk.buffer->getVertices())[idx2];

				//vector2df uv = mapper.calc(pf->faces[b_i].m_normal, vtx->Pos);

				mapper.calc(vtx);

				//std::cout << " " << vtx[0]->TCoords2.X << "," << vtx[0]->TCoords2.Y << "  ";
				//std::cout << " " << vtx[1]->TCoords2.X << "," << vtx[1]->TCoords2.Y << "  ";
				//std::cout << " " << vtx[2]->TCoords2.X << "," << vtx[2]->TCoords2.Y << "\n";

				/*
				if (uv_type == MAP_UVS_TEXTURE)
				{
					vtx->TCoords.set(uv.X, uv.Y);
				}
				else if (uv_type == MAP_UVS_LIGHTMAP)
				{
					vtx->TCoords2.set(uv.X, uv.Y);
					std::cout << uv.X << "," << uv.Y << "\n";
				}*/
			}
		}
	}
}

void apply_transform_to_uvs(MeshNode_Interface* mesh_node, const std::vector<int>& surface, int uv_type, matrix4 mat);

#endif