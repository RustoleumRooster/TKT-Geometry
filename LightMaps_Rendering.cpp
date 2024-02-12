#include <irrlicht.h>
#include "LightMaps.h"
#include "csg_classes.h"
#include "utils.h"
#include "geometry_scene.h"
#include "tolerances.h"
#include <sstream>
#include <Eigen/Dense>

using namespace irr;
using namespace core;

vector3df getBoundingFrustum(const vector3df* points, vector3df v0, vector3df& Up, vector3df center, f32& FOVout_x, f32& FOVout_y, LineHolder& graph)
{

	f32 R = 0;
	for (int i = 0; i < 4; i++)
	{
		R = fmax(R, vector3df(points[i] - center).getLength());
	}

	if (R < vector3df(center - v0).getLength())
	{
		//std::cout << "(outside radius)\n";
		f32 theta = asin(R / vector3df(center - v0).getLength());

		//graph.lines.push_back(line3df(center, center+(r*R)));

		FOVout_x = theta * 2;
		FOVout_y = theta * 2;

		return center - v0;
	}
	else
	{
		//std::cout << "(inside radius)\n";

		plane3df plane(points[0], points[1], points[2]);

		vector3df N = plane.Normal;
		vector3df v1;

		plane.getIntersectionWithLine(v0, N, v1);

		R = 0;
		for (int i = 0; i < 4; i++)
		{
			R = fmax(R, vector3df(points[i] - v1).getLength());
		}

		f32 theta = atan(R / vector3df(v1 - v0).getLength());

		//graph.lines.push_back(line3df(v1, v0));

		FOVout_x = theta * 2;
		FOVout_y = theta * 2;

		return v1 - v0;

	}
}

void GenLightMaps::calculate_optimal_view_frustum(vector3df v0, vector3df points[4], bool reverse_triangles, core::dimension2d<u32> texture_dimension, matrix4& proj_out, matrix4& view_out)
{
	vector3df center = points[0] + points[1] + points[2] + points[3];
	center /= 4;

	f32 fov_x = 0;
	f32 fov_y = 0;

	vector3df upvec(0, 1, 0);

	vector3df resv = getBoundingFrustum(points, v0, upvec, center, fov_x, fov_y, m_graph);
	resv.normalize();

	//std::cout << "FOV: " << fov_x << ", " << fov_y << "\n";

	matrix4 VIEW;
	VIEW.buildCameraLookAtMatrixLH(v0, v0 + resv * 100,
		is_parallel_normal(resv, vector3df(0.0, 1.0, 0.0)) ? vector3df(0.5, 1.0, 0.0) : vector3df(0.0, 1.0, 0.0));

	matrix4 PROJ;
	PROJ.buildProjectionMatrixPerspectiveFovLH(fmax(fov_y, fov_x), 1.0, 8, 2000);

	core::dimension2d<u32> dim = texture_dimension;//Texture->getOriginalSize();

	f32 X[4];
	f32 Y[4];
	f32 Z[4];
	f32 x[4];
	f32 y[4];
	f32 u[4];
	f32 v[4];
	f32 u_[4];
	f32 v_[4];
	f32 X_[4];
	f32 Y_[4];
	f32 Z_[4];

	//do initial map to view frustrum 
	{
		core::matrix4 trans;
		trans.setbyproduct_nocheck(PROJ, VIEW);

		dim.Width /= 2;
		dim.Height /= 2;

		for (int i = 0; i < 4; i++)
		{
			f32 transformedPos[4] = { points[i].X, points[i].Y, points[i].Z, 1.0f };

			trans.multiplyWith1x4Matrix(transformedPos);

			const f32 zDiv = transformedPos[3] == 0.0f ? 1.0f :
				core::reciprocal(transformedPos[3]);

			vector2di pos = vector2di(
				dim.Width + core::round32(dim.Width * (transformedPos[0] * zDiv)),
				dim.Height - core::round32(dim.Height * (transformedPos[1] * zDiv)));

			u[i] = pos.X;
			v[i] = pos.Y;

			X[i] = points[i].X - v0.X;
			Y[i] = points[i].Y - v0.Y;
			Z[i] = points[i].Z - v0.Z;

			X_[i] = X[i];
			Y_[i] = Y[i];
			Z_[i] = Z[i];
		}
	}

	dim = texture_dimension;//Texture->getOriginalSize();

	vector3df cp0 = vector3df(vector3df(u[1], 0, v[1]) - vector3df(u[0], 0, v[0]))
		.crossProduct(vector3df(vector3df(u[2], 0, v[2]) - vector3df(u[1], 0, v[1])));

	cp0.normalize();
	bool bReverse0 = cp0.dotProduct(vector3df(0, 1, 0)) < 0;

	if (bReverse0)
	{
		x[0] = 0;
		y[0] = 0;
		x[3] = 0;
		y[3] = dim.Height;
		x[2] = dim.Width;
		y[2] = dim.Height;
		x[1] = dim.Width;
		y[1] = 0;
	}
	else
	{
		x[0] = 0;
		y[0] = 0;
		x[1] = 0;
		y[1] = dim.Height;
		x[2] = dim.Width;
		y[2] = dim.Height;
		x[3] = dim.Width;
		y[3] = 0;
	}
	/*
	m_graph.lines.push_back(line3df(vector3df(x[0], 0, y[0]), vector3df(u[0], 0, v[0])));
	m_graph.lines.push_back(line3df(vector3df(x[1], 0, y[1]), vector3df(u[1], 0, v[1])));
	m_graph.lines.push_back(line3df(vector3df(x[2], 0, y[2]), vector3df(u[2], 0, v[2])));
	m_graph.lines.push_back(line3df(vector3df(x[3], 0, y[3]), vector3df(u[3], 0, v[3])));

	m_graph.lines.push_back(line3df(vector3df(x[0], 0, y[0]), vector3df(x[1], 0, y[1])));
	m_graph.lines.push_back(line3df(vector3df(x[1], 0, y[1]), vector3df(x[2], 0, y[2])));
	m_graph.lines.push_back(line3df(vector3df(x[2], 0, y[2]), vector3df(x[3], 0, y[3])));
	m_graph.lines.push_back(line3df(vector3df(x[0], 0, y[0]), vector3df(x[3], 0, y[3])));

	m_graph.lines.push_back(line3df(vector3df(u[0], 0, v[0]), vector3df(u[1], 0, v[1])));
	m_graph.lines.push_back(line3df(vector3df(u[1], 0, v[1]), vector3df(u[2], 0, v[2])));
	m_graph.lines.push_back(line3df(vector3df(u[2], 0, v[2]), vector3df(u[3], 0, v[3])));
	m_graph.lines.push_back(line3df(vector3df(u[0], 0, v[0]), vector3df(u[3], 0, v[3])));
	*/
	//match 3d point to viewport corners
	int K = 0;

	{
		f32 minlen = 9999;

		//std::cout << "\n";
		for (int j = 0; j < 4; j++)
		{
			//std::cout <<" j: " << j << " -> ";

			f32 len = sqrt((x[0] - u[j]) * (x[0] - u[j]) +
				(y[0] - v[j]) * (y[0] - v[j]));

			//std::cout << len << "\n";

			if (len < minlen)
			{
				minlen = len;
				K = j;
			}

		}
		//std::cout << " best j = " << K << " ";
		//std::cout << "\n";
	}

	for (int i = 0; i < 4; i++)
	{
		u16 j = (i + K) % 4;
		//std::cout << i << " -> " << j << " ";
		X[i] = X_[j];
		Y[i] = Y_[j];
		Z[i] = Z_[j];


		u_[i] = u[j];
		v_[i] = v[j];
	}

	/*
	if (false)
	{
		for (int i = 0; i < 4; i++)
		{
			std::cout << x[i] << ", " << y[i] << "  ->  " << u_[i] << ", " << v_[i] << "  ---> " <<
				X[i] << "," << Y[i] << "," << Z[i] <<
				" \n";
		}
	}*/

	if (reverse_triangles)
	{
		for (int i = 0; i < 4; i++)
		{
			y[i] = y[i] == 0 ? dim.Height : 0;
		}
	}


	if (false)
	{
		//std::cout << "points are:\n";
		for (int i = 0; i < 4; i++)
		{
			std::cout << x[i] << ", " << y[i] << "  ->  " << u_[i] << ", " << v_[i] << "  ---> " <<
				X[i] << "," << Y[i] << "," << Z[i] <<
				" \n";
		}
	}

	{
		//Use the SVD to solve for the Projection matrix which maps 3-D points to viewport corners
		Eigen::Matrix<double, 8, 9> A{
			{ X[0], Y[0], Z[0], 0, 0, 0, -x[0] * X[0], -x[0] * Y[0], -x[0] * Z[0] },
			{ 0, 0, 0, X[0], Y[0], Z[0], -y[0] * X[0], -y[0] * Y[0], -y[0] * Z[0] },
			{ X[1], Y[1], Z[1], 0, 0, 0, -x[1] * X[1], -x[1] * Y[1], -x[1] * Z[1] },
			{ 0, 0, 0, X[1], Y[1], Z[1], -y[1] * X[1], -y[1] * Y[1], -y[1] * Z[1] },
			{ X[2], Y[2], Z[2], 0, 0, 0, -x[2] * X[2], -x[2] * Y[2], -x[2] * Z[2] },
			{ 0, 0, 0, X[2], Y[2], Z[2], -y[2] * X[2], -y[2] * Y[2], -y[2] * Z[2] },
			{ X[3], Y[3], Z[3], 0, 0, 0, -x[3] * X[3], -x[3] * Y[3], -x[3] * Z[3] },
			{ 0, 0, 0, X[3], Y[3], Z[3], -y[3] * X[3], -y[3] * Y[3], -y[3] * Z[3] },
		};

		Eigen::JacobiSVD<Eigen::Matrix<double, 8, 9>, Eigen::ComputeFullV> svd(A);
		//std::cout << "Singular values are:" << std::endl << svd.singularValues() << std::endl;

		//Solution will be the last column in V, i.e. the column which doesn't correspond to any non-zero singular value
		Eigen::Vector<double, 9> p = svd.matrixV().col(8);

		//Construct the matrix Projection matrix P from the solution
		Eigen::Matrix<double, 3, 3> P{
			{ p[0], p[1], p[2] },
			{ p[3], p[4], p[5] },
			{ p[6], p[7], p[8] },
		};

		//normalize
		P /= sqrt(p[6] * p[6] + p[7] * p[7] + p[8] * p[8]);
		/*
		Eigen::Vector3d rvec = P * Eigen::Vector3d(X[0], Y[0], Z[0]);
		rvec /= rvec[2];
		std::cout << "vector 0 = \n" << rvec << "\n";

		rvec = P * Eigen::Vector3d(X[1], Y[1], Z[1]);
		rvec /= rvec[2];
		std::cout << "vector 1 = \n" << rvec << "\n";

		rvec = P * Eigen::Vector3d(X[2], Y[2], Z[2]);
		rvec /= rvec[2];
		std::cout << "vector 2 = \n" << rvec << "\n";

		rvec = P * Eigen::Vector3d(X[3], Y[3], Z[3]);
		rvec /= rvec[2];
		std::cout << "vector 3 = \n" << rvec << "\n";
		*/
		//construct the OpenGL Projection and View matrices from the Projection matrix P
		Eigen::Vector3d p0 = P.row(0);
		Eigen::Vector3d p1 = P.row(1);
		Eigen::Vector3d p2 = P.row(2);

		double Cx = p0.transpose() * p2;
		double Cy = p1.transpose() * p2;
		double a = -sqrt(p0.transpose() * p0 - (Cx * Cx));
		double b = -sqrt(p1.transpose() * p1 - (Cy * Cy));

		Eigen::Vector3d r0 = (p0 - (p2 * Cx)) / a;
		Eigen::Vector3d r1 = (p1 - (p2 * Cy)) / b;
		Eigen::Vector3d r2 = p2;

		matrix4 VIEW_ = core::matrix4(
			r0[0], r0[1], r0[2], (-r0[0] * v0.X) - (r0[1] * v0.Y) - (r0[2] * v0.Z),
			r1[0], r1[1], r1[2], (-r1[0] * v0.X) - (r1[1] * v0.Y) - (r1[2] * v0.Z),
			r2[0], r2[1], r2[2], (-r2[0] * v0.X) - (r2[1] * v0.Y) - (r2[2] * v0.Z),
			0.0f, 0.0f, 0.0f, 1.0f
		);

		//flip sign if necessary
		matrix4 VIEW_2;
		VIEW_.getTransposed(VIEW_2);
		vector3df shifted_center = center - v0;
		VIEW_2.rotateVect(shifted_center);

		if (shifted_center.Z > 0)
		{
			VIEW_ = core::matrix4(
				-r0[0], -r0[1], -r0[2], (r0[0] * v0.X) + (r0[1] * v0.Y) + (r0[2] * v0.Z),
				-r1[0], -r1[1], -r1[2], (r1[0] * v0.X) + (r1[1] * v0.Y) + (r1[2] * v0.Z),
				-r2[0], -r2[1], -r2[2], (r2[0] * v0.X) + (r2[1] * v0.Y) + (r2[2] * v0.Z),
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}

		f32 f = 1000;
		f32 n = 8;

		matrix4 PROJECTION_ = core::matrix4(
			-a * 2 / dim.Width, 0.0f, 1.0f - (Cx * 2 / dim.Width), 0.0f,
			0.0f, -b * 2 / dim.Height, 1.0f - (Cy * 2 / dim.Height), 0.0f,
			0.0f, 0.0f, -(f + n) / (f - n), -f * n * 2 / (f - n),
			0.0f, 0.0f, -1.0f, 0.0f
		);

		VIEW_.getTransposed(view_out);
		PROJECTION_.getTransposed(proj_out);
	}
}

void GenLightMaps::render_light(video::ITexture* render_texture, int f_j, const matrix4& view, const matrix4 proj)
{
	driver->setRenderTarget(render_texture, true, true, video::SColor(255, 0, 0, 0));

	driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
	driver->setTransform(video::ETS_PROJECTION, proj);
	driver->setTransform(video::ETS_VIEW, view);

	video::SMaterial someMaterial;
	someMaterial.Lighting = false;
	someMaterial.MaterialType = video::EMT_SOLID;

	video::SMaterial lightsMaterial;
	lightsMaterial.Lighting = false;
	lightsMaterial.MaterialType = m_DynamicLightMaterialType;
	lightsMaterial.setTexture(0, white_texture);

	for (int f_i = 0; f_i < geo_scene->get_total_geometry()->faces.size(); f_i++)
	{
		if (geo_scene->get_total_geometry()->faces[f_i].loops.size() > 0)
		{
			int index = geo_scene->edit_meshnode_interface.get_buffer_index_by_face(f_i);

			if (f_i == f_j)
			{
				geo_scene->getMeshNode()->render_special(index, lightsMaterial, white_texture);
			}
			else
				geo_scene->getMeshNode()->render_special(index, someMaterial, black_texture);
		}
	}

	//====
	/*
	driver->setRenderTarget(intermediate_texture, true, true, video::SColor(255, 0, 0, 0));

	driver->setTransform(video::ETS_PROJECTION, core::IdentityMatrix);
	driver->setTransform(video::ETS_VIEW, core::IdentityMatrix);
	driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);

	driver->setTransform(video::ETS_PROJECTION, PROJECTION);
	driver->setTransform(video::ETS_VIEW, VIEW);
	//geo_scene->getMeshNode()->render();

	lightsMaterial.BackfaceCulling = false;
	m_plate->render(driver, lightsMaterial);

	for (int f_i = 0; f_i < geo_scene->get_total_geometry()->faces.size(); f_i++)
	{
		if (geo_scene->get_total_geometry()->faces[f_i].loops.size() > 0)
		{
			int index = geo_scene->edit_meshnode_interface.get_buffer_index_by_face(f_i);
			if (f_i == f_j)
			{
				//geo_scene->getMeshNode()->render_special(index, lightsMaterial, white_texture);
				//geo_scene->getMeshNode()->render_special(index, someMaterial, white_texture);
				//geo_scene->getMeshNode()->SetFaceTexture(index, Texture);
			}
			//else
			//	geo_scene->getMeshNode()->render_special(index, someMaterial, black_texture);
		}
	}
	*/
	//====

	driver->setRenderTarget(NULL, true, true, video::SColor(0, 0, 0, 0));
}

bool GenLightMaps::light_intersects_quad(int f_j, vector3df v0, u16 Radius, const rect<f32>& face_bbox, LineHolder& graph)
{
	polyfold* pf = geo_scene->get_total_geometry();
	poly_face* f = &pf->faces[f_j];

	plane3df plane(pf->vertices[f->vertices[0]].V, f->m_normal);

	vector3df R;

	plane.getIntersectionWithLine(v0, f->m_normal, R);

	vector3df vr = R - v0;
	f32 r = vr.getLength();

	if (r > Radius)
		return false;

	f32 radius = sqrt(Radius * Radius - r * r);


	core::matrix4 mRot = f->get2Dmat();
	vector3df R_1 = R;

	mRot.rotateVect(R_1);

	//addCircle(vector3df(0,0,0), vector3df(0,1,0), radius, 16, graph);
	//addCircle(R_1, plane.Normal, radius, 16, graph);

	rect<f32> box = face_bbox;

	box.UpperLeftCorner -= vector2df(R_1.X, R_1.Z);
	box.LowerRightCorner -= vector2df(R_1.X, R_1.Z);

	vector3df z0 = vector3df(box.UpperLeftCorner.X, 0, box.UpperLeftCorner.Y);
	vector3df z1 = vector3df(box.UpperLeftCorner.X, 0, box.LowerRightCorner.Y);
	vector3df z2 = vector3df(box.LowerRightCorner.X, 0, box.LowerRightCorner.Y);
	vector3df z3 = vector3df(box.LowerRightCorner.X, 0, box.UpperLeftCorner.Y);

	//graph.lines.push_back(line3df(z0, z1));
	//graph.lines.push_back(line3df(z1, z2));
	//graph.lines.push_back(line3df(z2, z3));
	//graph.lines.push_back(line3df(z3, z0));


	f32 x, y, x0, y0, theta;

	x0 = box.UpperLeftCorner.X + 0.5 * (box.LowerRightCorner.X - box.UpperLeftCorner.X);
	y0 = box.UpperLeftCorner.Y + 0.5 * (box.LowerRightCorner.Y - box.UpperLeftCorner.Y);

	if ((box.isPointInside(core::vector2df(0, 0)) ||
		vector2df(x0, y0).getLength() < radius))
		return true;

	u16 nPointsInside = 0;

	y = box.UpperLeftCorner.Y;
	if (fabs(y) <= radius)
	{
		theta = asin(y / radius);
		x = cos(theta) * radius;
		graph.points.push_back(vector3df(x, 0, y0));
		graph.points.push_back(vector3df(-x, 0, y0));
		if (box.isPointInside(core::vector2df(x, y0)) || box.isPointInside(core::vector2df(-x, y0)))
			nPointsInside++;
	}

	y = box.LowerRightCorner.Y;
	if (fabs(y) <= radius)
	{
		theta = asin(y / radius);
		x = cos(theta) * radius;
		graph.points.push_back(vector3df(x, 0, y0));
		graph.points.push_back(vector3df(-x, 0, y0));
		if (box.isPointInside(core::vector2df(x, y0)) || box.isPointInside(core::vector2df(-x, y0)))
			nPointsInside++;
	}

	x = box.UpperLeftCorner.X;
	if (fabs(x) <= radius)
	{
		theta = acos(x / radius);
		y = sin(theta) * radius;
		graph.points.push_back(vector3df(x0, 0, y));
		graph.points.push_back(vector3df(x0, 0, -y));
		if (box.isPointInside(core::vector2df(x0, y)) || box.isPointInside(core::vector2df(x0, -y)))
			nPointsInside++;
	}

	x = box.LowerRightCorner.X;
	if (fabs(x) <= radius)
	{
		theta = acos(x / radius);
		y = sin(theta) * radius;
		graph.points.push_back(vector3df(x0, 0, y));
		graph.points.push_back(vector3df(x0, 0, -y));
		if (box.isPointInside(core::vector2df(x0, y)) || box.isPointInside(core::vector2df(x0, -y)))
			nPointsInside++;
	}
	//std::cout << "inside points = " << nPointsInside << "\n";

	return nPointsInside > 0 ? true : false;

	//if (ret) std::cout << "intersection\n";
	//else std::cout << "no instersevtion\n";

	/*
	graph.lines.push_back(line3df(points_out[0], points_out[1]));
	graph.lines.push_back(line3df(points_out[1], points_out[2]));
	graph.lines.push_back(line3df(points_out[2], points_out[3]));
	graph.lines.push_back(line3df(points_out[3], points_out[0]));
	*/
}

void GenLightMaps::get_bounding_quad(polyfold* pf, poly_face* f, rect<f32> bbox, vector3df* points)
{
	core::matrix4 R = f->get2Dmat();
	f32 use_Y;

	vector3df avec = pf->vertices[f->vertices[0]].V;
	R.rotateVect(avec);
	use_Y = avec.Y;

	points[0] = vector3df(bbox.UpperLeftCorner.X, use_Y, bbox.UpperLeftCorner.Y);
	points[1] = vector3df(bbox.UpperLeftCorner.X, use_Y, bbox.LowerRightCorner.Y);
	points[2] = vector3df(bbox.LowerRightCorner.X, use_Y, bbox.LowerRightCorner.Y);
	points[3] = vector3df(bbox.LowerRightCorner.X, use_Y, bbox.UpperLeftCorner.Y);

	R.makeInverse();

	R.rotateVect(points[0]);
	R.rotateVect(points[1]);
	R.rotateVect(points[2]);
	R.rotateVect(points[3]);
}

//bool GenLightMaps::

void GenLightMaps::render_face(vector3df v0, int f_j, video::IImage* lightmap_section, int Radius, const matrix4& ORTHO_VIEW, const matrix4& ORTHO_PROJ, vector3df* points, const rect<f32>& bbox_2D, LineHolder& graph)
{
	polyfold* pf = geo_scene->get_total_geometry();
	poly_face* f = &pf->faces[f_j];

	/*
	m0 = vector3df(circle_square.UpperLeftCorner.X, use_Y, circle_square.UpperLeftCorner.Y);
	m1 = vector3df(circle_square.UpperLeftCorner.X, use_Y, circle_square.LowerRightCorner.Y);
	m2 = vector3df(circle_square.LowerRightCorner.X, use_Y, circle_square.UpperLeftCorner.Y);
	m3 = vector3df(circle_square.LowerRightCorner.X, use_Y, circle_square.LowerRightCorner.Y);

	R_inv.rotateVect(m0);
	R_inv.rotateVect(m1);
	R_inv.rotateVect(m2);
	R_inv.rotateVect(m3);*/

	if (!light_intersects_quad(f_j, v0, Radius, bbox_2D, graph))
	{
		//std::cout << "no light\n";
		return;
	}

	f32 ortho_width = bbox_2D.getWidth();
	f32 ortho_height = bbox_2D.getHeight();

	get_bounding_quad(pf, f, bbox_2D, points);
	/*
	//std::cout << "ortho size: " << ortho_width << ", " << ortho_height << "\n";

	core::matrix4 mRot = f->get2Dmat();
	f32 use_Y;

	vector3df avec = pf->vertices[f->vertices[0]].V;
	mRot.rotateVect(avec);
	use_Y = avec.Y;

	matrix4 R_inv = mRot;
	R_inv.makeInverse();

	points[0] = vector3df(bbox_2D.UpperLeftCorner.X, use_Y, bbox_2D.UpperLeftCorner.Y);
	points[1] = vector3df(bbox_2D.UpperLeftCorner.X, use_Y, bbox_2D.LowerRightCorner.Y);
	points[2] = vector3df(bbox_2D.LowerRightCorner.X, use_Y, bbox_2D.LowerRightCorner.Y);
	points[3] = vector3df(bbox_2D.LowerRightCorner.X, use_Y, bbox_2D.UpperLeftCorner.Y);

	R_inv.rotateVect(points[0]);
	R_inv.rotateVect(points[1]);
	R_inv.rotateVect(points[2]);
	R_inv.rotateVect(points[3]);
	*/

	//graph.lines.push_back(line3df(points[0], points[1]));
	//graph.lines.push_back(line3df(points[1], points[2]));
	//graph.lines.push_back(line3df(points[2], points[3]));
	//graph.lines.push_back(line3df(points[3], points[0]));


	MeshBuffer_Chunk chunk = geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(f_j);

	//the orientation of the quadrilateral needs to match the orientation of the triangles, so check it
	video::S3DVertex2TCoords* vtx0;
	video::S3DVertex2TCoords* vtx1;
	video::S3DVertex2TCoords* vtx2;

	u16 idx0 = chunk.buffer->getIndices()[chunk.begin_i + 0];
	u16 idx1 = chunk.buffer->getIndices()[chunk.begin_i + 1];
	u16 idx2 = chunk.buffer->getIndices()[chunk.begin_i + 2];

	vtx0 = &((video::S3DVertex2TCoords*)chunk.buffer->getVertices())[idx0];
	vtx1 = &((video::S3DVertex2TCoords*)chunk.buffer->getVertices())[idx1];
	vtx2 = &((video::S3DVertex2TCoords*)chunk.buffer->getVertices())[idx2];

	vector3df cp = vector3df(vtx1->Pos - vtx0->Pos).crossProduct(vector3df(vtx2->Pos - vtx1->Pos));

	cp.normalize();
	bool bReverse = cp.dotProduct(f->m_normal) > 0;

	calculate_optimal_view_frustum(v0, points, bReverse, Texture->getOriginalSize(), m_PROJECTION, m_VIEW);

	/*
	core::matrix4 m(core::matrix4::EM4CONST_NOTHING);
	m.setbyproduct_nocheck(PROJECTION,
		VIEW);

	SViewFrustum frustum;

	frustum.cameraPosition = v0;
	frustum.setFrom(m,false);

	graph.lines.push_back(line3d(v0, frustum.getFarLeftDown()));
	graph.lines.push_back(line3d(v0, frustum.getFarRightDown()));
	graph.lines.push_back(line3d(v0, frustum.getFarLeftUp()));
	graph.lines.push_back(line3d(v0, frustum.getFarRightUp()));*/


	/*
	matrix4 trans = PROJECTION;
	trans *= VIEW;
	//std::cout << "test points:\n";
	for (int i = 0; i < 4; i++)
	{
		f32 transformedPos[4] = { points[i].X, points[i].Y, points[i].Z, 1.0f };

		trans.multiplyWith1x4Matrix(transformedPos);


		const f32 zDiv = transformedPos[3] == 0.0f ? 1.0f :
			core::reciprocal(transformedPos[3]);
		std::cout << zDiv << "\n";

		vector2di pos = vector2di(
			dim.Width*0.5 + core::round32(dim.Width*0.5 * (transformedPos[0] * zDiv)),
			dim.Height*0.5 - core::round32(dim.Height*0.5 * (transformedPos[1] * zDiv)));

	}*/

	render_light(Texture, f_j, m_VIEW, m_PROJECTION);
	/*
	matrix4 VIEW_0;
	VIEW_0.buildCameraLookAtMatrixLH(center + f->m_normal * 100, center,
		is_parallel_normal(f->m_normal, vector3df(0.0, 1.0, 0.0)) ? vector3df(0.5, 1.0, 0.0) : vector3df(0.0, 1.0, 0.0));

	f32 ortho_size = calculate_actual_ortho_size(VIEW_0, points, ortho_width, ortho_height);

	matrix4 PROJ_0;
	PROJ_0.buildProjectionMatrixOrthoLH(ortho_size, ortho_size, 0, 10000);

	core::matrix4 tcoord_transform;
	tcoord_transform.setbyproduct_nocheck(PROJ_0, VIEW_0);

	MeshBuffer_Chunk chunk1 = geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(f_j);
	//set_mesh_tcoords(chunk1, tcoord_transform, texture_block, ortho_width, ortho_height, lightmap_image->getDimension());
	set_mesh_tcoords(chunk1, tcoord_transform, texture_block, ortho_width, ortho_height, lightmap_dim);

	chunk1 = geo_scene->final_meshnode_interface.get_mesh_buffer_by_face(f_j);
	//set_mesh_tcoords(chunk1, tcoord_transform, texture_block, ortho_width, ortho_height, lightmap_image->getDimension());
	set_mesh_tcoords(chunk1, tcoord_transform, texture_block, ortho_width, ortho_height, lightmap_dim);
	*/

	core::dimension2d<u32> otsize(f->lightmap_res, f->lightmap_res);
	video::ITexture* ortho_texture = driver->addRenderTargetTexture(otsize, "rtt", irr::video::ECF_A8R8G8B8);

	driver->setRenderTarget(ortho_texture, true, true, video::SColor(255, 0, 0, 0));

	geo_scene->getMeshNode()->copyMaterials();

	lightmap_textures.push_back(ortho_texture);
	//std::cout << "set lightmap " << lightmap_textures.size() - 1 << "\n";
	//og_f->lightmap_index = lightmap_textures.size() - 1;

	driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
	driver->setTransform(video::ETS_VIEW, ORTHO_VIEW);
	driver->setTransform(video::ETS_PROJECTION, ORTHO_PROJ);

	video::SMaterial someMaterial;
	someMaterial.Lighting = false;
	someMaterial.MaterialType = m_projectionMaterialType;

	int index = geo_scene->edit_meshnode_interface.get_buffer_index_by_face(f_j);
	geo_scene->getMeshNode()->render_special(index, someMaterial, Texture);

	//============
	/*
	driver->setRenderTarget(intermediate_texture, true, true, video::SColor(255, 0, 0, 0));

	driver->setTransform(video::ETS_PROJECTION, core::IdentityMatrix);
	driver->setTransform(video::ETS_VIEW, core::IdentityMatrix);
	driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);

	camera_ortho->render();

	{
		int index = geo_scene->edit_meshnode_interface.get_buffer_index_by_face(f_j);

		geo_scene->getMeshNode()->render_special(index, someMaterial, Texture);
		//geo_scene->getMeshNode()->SetFaceTexture(index, ortho_texture);
	}*/
	//===============

	driver->setRenderTarget(NULL, true, true, video::SColor(0, 0, 0, 0));

	video::IImage* src_img = driver->createImage(ortho_texture, vector2di(0, 0), otsize);

	video::SColor color_a, color_b;
	for (int i = 0; i < otsize.Width; i++)
		for (int j = 0; j < otsize.Height; j++)
		{
			color_a = src_img->getPixel(i, j);
			color_b = lightmap_section->getPixel(i, j);

			color_b.set(255,
				std::min((u32)255, color_a.getRed() + color_b.getRed()),
				std::min((u32)255, color_a.getGreen() + color_b.getGreen()),
				std::min((u32)255, color_a.getBlue() + color_b.getBlue()));

			lightmap_section->setPixel(i, j, color_b);
		}

	src_img->drop();

	//std::cout << "\n";
}

int* GenLightMaps::GetNTriangles()
{
	return &m_nTriangles;
}

core::vector3df* GenLightMaps::GetVertices()
{
	return m_Vertices;
}

core::vector3df* GenLightMaps::GetNormals()
{
	return m_Normals;
}
/*
void GenLightMaps::render_face2(core::vector3df v0, int f_j, video::IImage* lightmap_section, int Radius, const core::matrix4& ORTHO_VIEW, const core::matrix4& ORTHO_PROJ, core::vector3df* points, const core::rect<f32>& bbox_2D, LineHolder& graph)
{
	core::dimension2d<u32> otsize(f->lightmap_res, f->lightmap_res);
	video::ITexture* ortho_texture = driver->addRenderTargetTexture(otsize, "rtt", irr::video::ECF_A8R8G8B8);

	driver->setRenderTarget(ortho_texture, true, true, video::SColor(255, 0, 0, 0));

	geo_scene->getMeshNode()->copyMaterials();

	lightmap_textures.push_back(ortho_texture);

	driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
	driver->setTransform(video::ETS_VIEW, ORTHO_VIEW);
	driver->setTransform(video::ETS_PROJECTION, ORTHO_PROJ);

	video::SMaterial someMaterial;
	someMaterial.Lighting = false;
	someMaterial.MaterialType = m_PathTracingMaterialType;

	int index = geo_scene->edit_meshnode_interface.get_buffer_index_by_face(f_j);
	geo_scene->getMeshNode()->render_special(index, someMaterial, Texture);

	driver->setRenderTarget(NULL, true, true, video::SColor(0, 0, 0, 0));
}*/

