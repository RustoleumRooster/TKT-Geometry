#ifndef _UV_MAPPING_H_
#define _UV_MAPPING_H_

#include <irrlicht.h>
#include "csg_classes.h"
#include "BufferManager.h"
#include "geometry_scene.h"
#include <algorithm>

using namespace irr;
using namespace core;
using namespace std;

class map_face_to_uvs2
{
public:
	vector3df verts[4];
	vector3df u_vec;
	vector3df v_vec;
	f32 x_len;
	f32 y_len;

	u16 m_uv_width = 0;
	u16 m_uv_height = 0;
	u16 column_width = 0;
	u16 column_height = 0;

	f32 max_height() { return vector3df(verts[3] - verts[3]).getLength(); }
	f32 max_width() { return vector3df(verts[1] - verts[0]).getLength(); }

	void init(const poly_face* f, int v0_idx, u16 pixel_width, u16 pixel_height)
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

		m_uv_width = pixel_width;
		m_uv_height = pixel_height;

		column_height = m_uv_height - 3;
		column_width = m_uv_width - 3;

		x_len = vector3df(verts[1] - verts[0]).getLength();
		y_len = vector3df(verts[3] - verts[0]).getLength();
	}

	void calc(video::S3DVertex2TCoords** V, int f_i) const
	{
		u16 x0 = 1;
		u16 y0 = 1;

		V[0]->TCoords2.X = x0 + (vector3df(V[0]->Pos - verts[0]).dotProduct(u_vec) / x_len * column_width);
		V[0]->TCoords2.Y = y0 + (vector3df(V[0]->Pos - verts[0]).dotProduct(v_vec) / y_len * column_height);

		V[1]->TCoords2.X = x0 + (vector3df(V[1]->Pos - verts[0]).dotProduct(u_vec) / x_len * column_width);
		V[1]->TCoords2.Y = y0 + (vector3df(V[1]->Pos - verts[0]).dotProduct(v_vec) / y_len * column_height);

		V[2]->TCoords2.X = x0 + (vector3df(V[2]->Pos - verts[0]).dotProduct(u_vec) / x_len * column_width);
		V[2]->TCoords2.Y = y0 + (vector3df(V[2]->Pos - verts[0]).dotProduct(v_vec) / y_len * column_height);
	}

};

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

	void calc(video::S3DVertex2TCoords** V, int f_i) const
	{
		V[0]->TCoords2.X = vector3df(V[0]->Pos - verts[0]).dotProduct(u_vec) / vector3df(verts[1] - verts[0]).getLength();
		V[0]->TCoords2.Y = vector3df(V[0]->Pos - verts[0]).dotProduct(v_vec) / vector3df(verts[3] - verts[0]).getLength();

		V[1]->TCoords2.X = vector3df(V[1]->Pos - verts[0]).dotProduct(u_vec) / vector3df(verts[1] - verts[0]).getLength();
		V[1]->TCoords2.Y = vector3df(V[1]->Pos - verts[0]).dotProduct(v_vec) / vector3df(verts[3] - verts[0]).getLength();

		V[2]->TCoords2.X = vector3df(V[2]->Pos - verts[0]).dotProduct(u_vec) / vector3df(verts[1] - verts[0]).getLength();
		V[2]->TCoords2.Y = vector3df(V[2]->Pos - verts[0]).dotProduct(v_vec) / vector3df(verts[3] - verts[0]).getLength();

	}

};


class map_cylinder_to_uv2
{

public:

	vector3df iY, v0, v1, r0;
	f32 m_height, m_radius;

	u16 m_uv_width = 0;
	u16 m_uv_height = 0;

	vector2df m_min, m_max;
	int n = 0;

	f32 uv_width() { return m_max.X - m_min.X; }
	f32 uv_height() { return m_max.Y - m_min.Y; }

	u16 n_columns = 0;
	u16 column_width = 0;
	u16 column_height = 0;

	polyfold* m_pf;



	void init(const surface_group* sg, polyfold* pf, int sfg_i, u16 pixel_width, u16 pixel_height)
	{
		r0 = sg->point;
		v0 = sg->vec;
		v1 = sg->vec1;
		m_height = sg->height;
		m_radius = sg->radius;

		iY = v0.crossProduct(v1);
		iY.normalize();
		n = 0;

		n_columns = 0;

		for (const poly_face& f: pf->faces)
		{
			if (f.surface_group == sfg_i)
			{
				n_columns += 1;
			}
		}

		m_uv_width = pixel_width;
		m_uv_height = pixel_height;

		column_width = ( (pixel_width - 3) / n_columns ) - 1;
		column_height = pixel_height - 3;

		m_pf = pf;
	}

	void calc(video::S3DVertex2TCoords** V, int f_i)
	{
		vector3df face_normal = vector3df(V[2]->Pos - V[0]->Pos).crossProduct(vector3df(V[1]->Pos - V[0]->Pos));
		const poly_face& f = m_pf->faces[f_i];
		u16 x0 = 1 + (f.column * (column_width+1));
		u16 y0 = 1;

		//std::cout << f.column << "  " << x0 << "\n";

		vector3df verts[4];
		vector3df u_vec;
		vector3df v_vec;

		f.get3DBoundingQuad(verts, 0);

		u_vec = verts[1] - verts[0];
		v_vec = verts[3] - verts[0];

		u_vec.normalize();
		v_vec.normalize();

		u_vec.X = fabs(u_vec.X) < 0.0001 ? 0 : u_vec.X;
		u_vec.Y = fabs(u_vec.Y) < 0.0001 ? 0 : u_vec.Y;
		v_vec.X = fabs(v_vec.X) < 0.0001 ? 0 : v_vec.X;
		v_vec.Y = fabs(v_vec.Y) < 0.0001 ? 0 : v_vec.Y;

		f32 x_len = vector3df(verts[1] - verts[0]).getLength();
		f32 y_len = vector3df(verts[3] - verts[0]).getLength();

		V[0]->TCoords2.X = x0 + (vector3df(V[0]->Pos - verts[0]).dotProduct(u_vec) / x_len * column_width);
		V[0]->TCoords2.Y = y0 + (vector3df(V[0]->Pos - verts[0]).dotProduct(v_vec) / y_len * column_height);

		V[1]->TCoords2.X = x0 + (vector3df(V[1]->Pos - verts[0]).dotProduct(u_vec) / x_len * column_width);
		V[1]->TCoords2.Y = y0 + (vector3df(V[1]->Pos - verts[0]).dotProduct(v_vec) / y_len * column_height);

		V[2]->TCoords2.X = x0 + (vector3df(V[2]->Pos - verts[0]).dotProduct(u_vec) / x_len * column_width);
		V[2]->TCoords2.Y = y0 + (vector3df(V[2]->Pos - verts[0]).dotProduct(v_vec) / y_len * column_height);

		//cout << f_i << ":\n";
		//cout << V[0]->TCoords2.X << "," << V[0]->TCoords2.Y << "\n";
		//cout << V[1]->TCoords2.X << "," << V[1]->TCoords2.Y << "\n";
		//cout << V[2]->TCoords2.X << "," << V[2]->TCoords2.Y << "\n";
	}
};

class map_sphere_to_uv2
{

public:

	vector3df iY, v0, v1, r0;
	//f32 m_height, m_radius;

	u16 m_uv_width = 0;
	u16 m_uv_height = 0;

	vector2df m_min, m_max;
	int n = 0;

	f32 uv_width() { return m_max.X - m_min.X; }
	f32 uv_height() { return m_max.Y - m_min.Y; }

	u16 n_rows = 0;
	vector<u16> rows;
	vector<u16> n_columns;
	vector<u16> column_width;
	u16 column_height = 0;

	polyfold* m_pf;

	int findRowPos( u16 row_no) 
	{
		for (int i = 0; i < rows.size(); i++)
		{
			if (rows[i] == row_no)
				return i;
		}
		return -1;
	};


	void init(const surface_group* sg, polyfold* pf, const vector<int> surface, int sfg_i, u16 pixel_width, u16 pixel_height)
	{
		r0 = sg->point;
		v0 = sg->vec;
		v1 = sg->vec1;
		//m_height = sg->height;
		//m_radius = sg->radius;

		iY = v0.crossProduct(v1);
		iY.normalize();
		n = 0;

		for (int b_i : surface)
		{
			bool b = false;
			for (u16 j : rows)
			{
				if (j == pf->faces[b_i].row)
					b = true;
			}
			if (!b)
			{
				rows.push_back(pf->faces[b_i].row);
			}
		}
		n_rows = rows.size();

		n_columns.resize(n_rows);
		column_width.resize(n_rows);

		for (int b_i : surface)
		{
			int row_pos = findRowPos( pf->faces[b_i].row);
			n_columns[row_pos] += 1;
		}

		for (int i = 0; i < n_rows; i++)
		{
			column_width[i] = ((pixel_width - 3) / n_columns[i]) - 1;
		}

		column_height = ((pixel_width - 3) / n_rows) - 1;

		m_uv_width = pixel_width;
		m_uv_height = pixel_height;

		//column_width = ((pixel_width - 3) / n_columns) - 1;
		//column_height = pixel_height - 3;

		m_pf = pf;
	}

	void calc(video::S3DVertex2TCoords** V, int f_i)
	{
		vector3df face_normal = vector3df(V[2]->Pos - V[0]->Pos).crossProduct(vector3df(V[1]->Pos - V[0]->Pos));
		const poly_face& f = m_pf->faces[f_i];

		int row_pos = findRowPos(f.row);
		int col_width = column_width[row_pos];

		u16 x0 = 1 + (f.column * (col_width + 1));
		u16 y0 = 1 + (row_pos * (column_height + 1));

		//std::cout << f.column << "  " << x0 << "\n";

		vector3df verts[4];
		vector3df u_vec;
		vector3df v_vec;

		f.get3DBoundingQuad(verts, 0);

		u_vec = verts[1] - verts[0];
		v_vec = verts[3] - verts[0];

		u_vec.normalize();
		v_vec.normalize();

		u_vec.X = fabs(u_vec.X) < 0.0001 ? 0 : u_vec.X;
		u_vec.Y = fabs(u_vec.Y) < 0.0001 ? 0 : u_vec.Y;
		v_vec.X = fabs(v_vec.X) < 0.0001 ? 0 : v_vec.X;
		v_vec.Y = fabs(v_vec.Y) < 0.0001 ? 0 : v_vec.Y;

		f32 x_len = vector3df(verts[1] - verts[0]).getLength();
		f32 y_len = vector3df(verts[3] - verts[0]).getLength();

		V[0]->TCoords2.X = x0 + (vector3df(V[0]->Pos - verts[0]).dotProduct(u_vec) / x_len * col_width);
		V[0]->TCoords2.Y = y0 + (vector3df(V[0]->Pos - verts[0]).dotProduct(v_vec) / y_len * column_height);

		V[1]->TCoords2.X = x0 + (vector3df(V[1]->Pos - verts[0]).dotProduct(u_vec) / x_len * col_width);
		V[1]->TCoords2.Y = y0 + (vector3df(V[1]->Pos - verts[0]).dotProduct(v_vec) / y_len * column_height);

		V[2]->TCoords2.X = x0 + (vector3df(V[2]->Pos - verts[0]).dotProduct(u_vec) / x_len * col_width);
		V[2]->TCoords2.Y = y0 + (vector3df(V[2]->Pos - verts[0]).dotProduct(v_vec) / y_len * column_height);

		//cout << f_i << ":\n";
		//cout << V[0]->TCoords2.X << "," << V[0]->TCoords2.Y << "\n";
		//cout << V[1]->TCoords2.X << "," << V[1]->TCoords2.Y << "\n";
		//cout << V[2]->TCoords2.X << "," << V[2]->TCoords2.Y << "\n";
	}
};

class map_sphere_to_uv3
{

public:

	const surface_group* sfg = NULL;

	vector3df iY, v0, v1, r0;

	u16 m_uv_width = 0;
	u16 m_uv_height = 0;

	vector2df m_min, m_max;
	int n = 0;

	f32 uv_width() { return m_max.X - m_min.X; }
	f32 uv_height() { return m_max.Y - m_min.Y; }

	u16 n_rows = 0;
	vector<u16> rows;
	vector<u16> n_columns;
	vector<u16> column_width;
	vector<f32> row_theta_upper;
	vector<f32> row_theta_lower;
	u16 column_height = 0;

	polyfold* m_pf;

	bool I_AM_A_COLUMN = false;

	int findRowPos(u16 row_no)
	{
		for (int i = 0; i < rows.size(); i++)
		{
			if (rows[i] == row_no)
				return i;
		}
		return -1;
		return row_no;
	};

	f32 calc_ypos_sphere(vector3df r)
	{
		vector3df rr = r - sfg->point;
		rr.normalize();
		return acos(rr.dotProduct(sfg->vec));
	}

	f32 calc_ypos_column(vector3df r)
	{
		vector3df rr = r - sfg->point;
		return rr.dotProduct(sfg->vec);
	}


	void init(const surface_group* sg, polyfold* pf, const vector<int> surface, int sfg_i, u16 pixel_width, u16 pixel_height, bool is_column)
	{
		r0 = sg->point;
		v0 = sg->vec;
		v1 = sg->vec1;

		I_AM_A_COLUMN = is_column;

		sfg = sg;

		iY = v0.crossProduct(v1);
		iY.normalize();
		n = 0;
		n_rows = 0;
		int first_row = 9999;
		int last_row = 0;
		for (int b_i : surface)
		{/*
			bool b = false;
			for (u16 j : rows)
			{
				if (j == pf->faces[b_i].row)
					b = true;
			}
			if (!b)*/
			{
				//rows.push_back(pf->faces[b_i].row);
				first_row = min(pf->faces[b_i].row, first_row);
				last_row = max(pf->faces[b_i].row, last_row);
				//n_rows++;
			}
		}
		//std::cout << " ROWS are " << first_row << " to " << last_row << "\n";
		rows.resize(1 + last_row - first_row);
		n_rows = rows.size();
		for (int i=0; i < n_rows; i++)
		{
			rows[i] = first_row + i;
			//std::cout << "ROW " << i <<" = " << rows[i] << "\n";
		}
		vector<vector<u16>> row_columns;

		row_columns.resize(n_rows);
		n_columns.assign(n_rows,0);
		column_width.resize(n_rows);
		row_theta_upper.assign(n_rows, PI);
		row_theta_lower.assign(n_rows, 0);

		for (int b_i : surface)
		{
			int row_pos = findRowPos(pf->faces[b_i].row);
			row_columns[row_pos].push_back(b_i);

			n_columns[row_pos] += 1;

			for (int v_i : pf->faces[b_i].vertices)
			{
				//vector3df r = pf->vertices[v_i].V - sfg->point;
				//r.normalize();
				//f32 theta = acos(r.dotProduct(sfg->vec));

				//f32 theta = r.dotProduct(sfg->vec);
				f32 theta;
				if (I_AM_A_COLUMN)
					theta = calc_ypos_column(pf->vertices[v_i].V);
				else
					theta = calc_ypos_sphere(pf->vertices[v_i].V);

				row_theta_upper[row_pos] = fmin(theta, row_theta_upper[row_pos]);
				row_theta_lower[row_pos] = fmax(theta, row_theta_lower[row_pos]);
				//std::cout << row_pos << theta << "\n"; // ": " << row_theta_upper[row_pos] << " to " << row_theta_lower[row_pos] << "\n";
			}
		}

		for (int i = 0; i < row_columns.size(); i++)
		{
			for (int j = 0; j < row_columns[i].size(); j++)
			{
				u16 f = row_columns[i][j];
			}

			std::sort(row_columns[i].begin(), row_columns[i].end(),
				[&](u16& a, u16& b)
				{
					return (pf->faces[a].column < pf->faces[b].column);
				});

			for (int j = 0; j < row_columns[i].size(); j++)
			{
				u16 f = row_columns[i][j];
				pf->faces[f].column = j;
			}
		}
		
		for (int i = 0; i < n_rows; i++)
		{
			column_width[i] = (pixel_width - 3 - (n_columns[i] - 1));
			//std::cout << row_theta_upper[i] << " - " << row_theta_lower[i] << "\n";
		}

		column_height = ((pixel_height - 3) / n_rows) - 1;

		m_uv_width = pixel_width;
		m_uv_height = pixel_height;

		//cout << "pixel_width: "<< pixel_width << "\n";
		//cout << "pixel_height: " << pixel_height << "\n";

		m_pf = pf;
	}

	void calc(video::S3DVertex2TCoords** V, int f_i)
	{
		vector3df face_normal = vector3df(V[2]->Pos - V[0]->Pos).crossProduct(vector3df(V[1]->Pos - V[0]->Pos));
		const poly_face& f = m_pf->faces[f_i];

		int row_pos = findRowPos(f.row);

		u16 x_offset = 1 + f.column;
		u16 y0 = 1 + (row_pos * (column_height + 1));

		//std::cout << f.row<< " " <<f.column << ":  " << "\n";

		int recalc = -1;

		for (int i = 0; i < 3; i++)
		{
			core::vector3df r = V[i]->Pos - sfg->point;
			r.normalize();
			core::vector3df rr = sfg->vec.crossProduct(r);
			rr.normalize();

			//f32 alpha = acos(r.dotProduct(sfg->vec));
			//f32 alpha = vector3df(V[i]->Pos - sfg->point).dotProduct(sfg->vec);
			f32 alpha;

			if (I_AM_A_COLUMN)
				alpha = calc_ypos_column(V[i]->Pos);
			else
				alpha = calc_ypos_sphere(V[i]->Pos);

			vector2df k;

			k.X = r.dotProduct(sfg->vec1);
			k.Y = -r.dotProduct(iY);

			f32 az = k.getAngleTrig();
			
			if (fabs(az - 360.0f) < 0.001f || fabs(az) < 0.001f)
			{
				if (face_normal.dotProduct(iY) * r.dotProduct(face_normal) > 0)
				{
					az = 360.0f;
				}
				else
				{
					az = 0.0f;
				}
			}

			if (fabs(fabs(r.dotProduct(sfg->vec)) - 1.0f) < 0.0001f)
			{
				recalc = i;
			}
			f32 xpos = x_offset + (az / 360.0f) * column_width[row_pos];
			//f32 ypos = y0 + column_height - (alpha - row_theta_lower[row_pos]) / (row_theta_upper[row_pos] - row_theta_lower[row_pos]) * column_height;
			f32 ypos = y0 + (alpha - row_theta_lower[row_pos]) / (row_theta_upper[row_pos] - row_theta_lower[row_pos]) * column_height;
			//cout << " " << xpos << "," << ypos << "\n";

			V[i]->TCoords2 = vector2df(xpos, ypos);
			//V[i]->TCoords2 = vector2df(xpos,ypos);

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

	void calc(video::S3DVertex2TCoords** V, int f_i)
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

	void calc(video::S3DVertex2TCoords** V, int f_i)
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
					az = 0.0f;
				}
				else
				{
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
void map_uvs(GeometryStack* geo_node, MeshNode_Interface* mesh_node, const std::vector<int>& surface, map_type& mapper, int uv_type)
{
	polyfold* pf = geo_node->get_total_geometry();

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

				mapper.calc(vtx, b_i);

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