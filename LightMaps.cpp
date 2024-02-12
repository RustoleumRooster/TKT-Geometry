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


GenLightMaps* LightMaps_Tool::m_lightmaps = NULL;

#define MAT_COUT(M) std::cout<<M[0]<<" "<<M[1]<<" "<<M[2]<<" "<<M[3]<<"\n"; \
					std::cout<<M[4]<<" "<<M[5]<<" "<<M[6]<<" "<<M[7]<<"\n"; \
					std::cout<<M[8]<<" "<<M[9]<<" "<<M[10]<<" "<<M[11]<<"\n"; \
					std::cout<<M[12]<<" "<<M[13]<<" "<<M[14]<<" "<<M[15]<<"\n"; 


void LightMap_Plate::set_buffer(const scene::SViewFrustum& frustum, core::plane3df plane)
{
	if (!Buffer)
	{

		Buffer = new scene::SMeshBuffer();
		Buffer->Material.Lighting = false;
		Buffer->Material.setTexture(0, m_texture);
		Buffer->Material.MaterialType = video::EMT_SOLID;

		Buffer->Vertices.set_used(4);
		Buffer->Indices.set_used(6);

		Buffer->Indices[0] = (u16)0;
		Buffer->Indices[1] = (u16)2;
		Buffer->Indices[2] = (u16)1;
		Buffer->Indices[3] = (u16)0;
		Buffer->Indices[4] = (u16)3;
		Buffer->Indices[5] = (u16)2;
	}

	core::vector3df V[4];
	core::vector3df camPos = frustum.cameraPosition;

	plane.getIntersectionWithLine(camPos, frustum.getNearLeftUp() - camPos, V[0]);
	plane.getIntersectionWithLine(camPos, frustum.getNearRightUp() - camPos, V[1]);
	plane.getIntersectionWithLine(camPos, frustum.getNearRightDown() - camPos, V[2]);
	plane.getIntersectionWithLine(camPos, frustum.getNearLeftDown() - camPos, V[3]);

	Buffer->Vertices[0].Pos = V[0];
	Buffer->Vertices[0].Color = video::SColor(255, 255, 255, 255);
	Buffer->Vertices[0].Normal = plane.Normal;
	Buffer->Vertices[0].TCoords = core::vector2df(1, 1);

	Buffer->Vertices[1].Pos = V[1];
	Buffer->Vertices[1].Color = video::SColor(255, 255, 255, 255);
	Buffer->Vertices[1].Normal = plane.Normal;
	Buffer->Vertices[1].TCoords = core::vector2df(1, 0);

	Buffer->Vertices[2].Pos = V[2];
	Buffer->Vertices[2].Color = video::SColor(255, 255, 255, 255);
	Buffer->Vertices[2].Normal = plane.Normal;
	Buffer->Vertices[2].TCoords = core::vector2df(0, 0);

	Buffer->Vertices[3].Pos = V[3];
	Buffer->Vertices[3].Color = video::SColor(255, 255, 255, 255);
	Buffer->Vertices[3].Normal = plane.Normal;
	Buffer->Vertices[3].TCoords = core::vector2df(0, 1);

}

void LightMap_Plate::render(video::IVideoDriver* driver, video::SMaterial& material)
{
	if (!driver)
		return;

	core::matrix4 mat;

	driver->setTransform(video::ETS_WORLD, mat);
	//driver->setMaterial(Buffer->Material);

	//if (texture)
	//material.setTexture(0, texture);
	//material.setTexture(0,mb->getMaterial().getTexture(0));

	driver->setMaterial(material);

	driver->drawVertexPrimitiveList(Buffer->getVertices(), 4,
		Buffer->getIndices(), 2, video::EVT_STANDARD, EPT_TRIANGLES, Buffer->getIndexType());
}

void addCircle(vector3df v0, vector3df N,  f32 radius, int nSegments, LineHolder& graph)
{
	f32 theta = 0;

	vector3df X = is_parallel_normal(N,vector3df(0,1,0)) ? 
						vector3df(1,0,0) 
						: N.crossProduct(vector3df(0, 1, 0));

	X.normalize();

	vector3df Y = N.crossProduct(X);
	Y.normalize();

	vector3df r = X * radius;

	f32 inc = 3.1415926 * 2 / nSegments;

	for (int i = 0; i < nSegments; i++)
	{
		theta += inc;

		f32 x = radius * cos(theta);
		f32 y = radius * sin(theta);

		line3df segment(v0+r, v0+ (X * x + Y * y));
		graph.lines.push_back(segment);
		
		r = X * x + Y * y;
	}

}

GenLightMaps::GenLightMaps(geometry_scene* g_scene_, scene::ISceneManager* smgr_, video::IVideoDriver* driver_, video::E_MATERIAL_TYPE projectionMaterialType)
{
	smgr = smgr_;
	geo_scene = g_scene_;
	driver = driver_;
	m_projectionMaterialType = projectionMaterialType;

	video::IImage* img = makeSolidColorImage(driver, video::SColor(0, 0, 0, 0));
	black_texture = driver->addTexture(irr::io::path("hit_color"), img);
	img->drop();

	img = makeSolidColorImage(driver, video::SColor(255, 255, 255, 255));
	white_texture = driver->addTexture(irr::io::path("hit_color"), img);
	img->drop();
}

GenLightMaps::~GenLightMaps()
{
	if (m_plate)
		delete m_plate;
}

void GenLightMaps::init()
{

	//std::cout << "Generating New Lightmaps\n";

	for (int f_j = 0; f_j < geo_scene->get_total_geometry()->faces.size(); f_j++)
	{
		if (geo_scene->get_total_geometry()->faces[f_j].loops.size() > 0)
		{
			int index = geo_scene->edit_meshnode_interface.get_buffer_index_by_face(f_j);
			scene::IMeshBuffer* buffer = geo_scene->getMeshNode()->getMesh()->getMeshBuffer(index);
			buffer->getMaterial().setTexture(1, NULL);
		}
	}

	geo_scene->getMeshNode()->copyMaterials();

	for (video::ITexture* tex : lightmap_textures)
	{
		driver->removeTexture(tex);
	}

	for (video::ITexture* tex : lightmap_textures2)
	{
		driver->removeTexture(tex);
	}

	lightmap_textures.clear();
	lightmap_textures2.clear();

	m_graph.lines.clear();


	if (Texture)
		driver->removeTexture(Texture);

	core::dimension2d<u32> tsize(512, 512);
	Texture = driver->addRenderTargetTexture(tsize, "rtt", irr::video::ECF_A8R8G8B8);

	intermediate_texture = driver->addRenderTargetTexture(tsize, "rtt", irr::video::ECF_A8R8G8B8);

	m_plate = new LightMap_Plate;
	

	//std::cout<<Texture<<"\n";
}

f32 GenLightMaps::calculate_actual_ortho_size(const matrix4& VIEW, vector3df points[4], u16 ortho_width, u16 ortho_height)
{
	matrix4 M;

	f32 ortho_size = fmax(ortho_width, ortho_height);

	matrix4 PROJ;
	PROJ.buildProjectionMatrixOrthoLH(ortho_size, ortho_size, 0, 10000);
	
	core::matrix4 transform;
	transform.setbyproduct_nocheck(PROJ, VIEW);

	dimension2du dim(ortho_size, ortho_size);

	dim.Width /= 2;
	dim.Height /= 2;

	int xmin = 9999;
	int xmax = -9999;
	int ymax = -9999;
	int ymin = 9999;

	for (int i = 0; i < 4; i++)
	{
		f32 transformedPos[4] = { points[i].X, points[i].Y, points[i].Z, 1.0f };

		transform.multiplyWith1x4Matrix(transformedPos);

		const f32 zDiv = transformedPos[3] == 0.0f ? 1.0f :
			core::reciprocal(transformedPos[3]);

		vector2di pos = vector2di(
			dim.Width + core::round32(dim.Width * (transformedPos[0] * zDiv)),
			dim.Height - core::round32(dim.Height * (transformedPos[1] * zDiv)));

		xmin = std::min(xmin, pos.X);
		xmax = std::max(xmax, pos.X);
		ymin = std::min(ymin, pos.Y);
		ymax = std::max(ymax, pos.Y);
	}
	ortho_height = ymax - ymin;
	ortho_width = xmax - xmin;

	return fmax(xmax - xmin, ymax - ymin);
}


void GenLightMaps::set_mesh_tcoords(const MeshBuffer_Chunk& chunk, const matrix4& transform, core::rect<u16> texture_block, u16 ortho_width, u16 ortho_height, dimension2du lightmap_dim)
{
	dimension2di dim = dimension2di(ortho_width, ortho_height);
	dim.Width /= 2;
	dim.Height /= 2;

	video::S3DVertex2TCoords* vtx;

	for (u16 i = chunk.begin_i; i < chunk.end_i; i++)
	{
		u16 idx = chunk.buffer->getIndices()[i];
		vtx = &((video::S3DVertex2TCoords*)chunk.buffer->getVertices())[idx];

		f32 transformedPos[4] = { vtx->Pos.X, vtx->Pos.Y, vtx->Pos.Z, 1.0f };

		transform.multiplyWith1x4Matrix(transformedPos);

		const f32 zDiv = transformedPos[3] == 0.0f ? 1.0f :
			core::reciprocal(transformedPos[3]);

		vector2di pos = vector2di(
			dim.Width + core::round32(dim.Width * (transformedPos[0] * zDiv)),
			dim.Height - core::round32(dim.Height * (transformedPos[1] * zDiv)));

		//vtx->TCoords2.set(pos.X/ortho_width, 1.0 - pos.Y/ortho_height);

		f32 xcoord = ((f32)pos.X / ortho_width) * texture_block.getWidth() / lightmap_dim.Width;
		xcoord += (f32)texture_block.UpperLeftCorner.X / lightmap_dim.Width;

		f32 ycoord = ((f32)pos.Y / ortho_height) * texture_block.getHeight() / lightmap_dim.Height;;
		ycoord += (f32)texture_block.UpperLeftCorner.Y / lightmap_dim.Height;

		vtx->TCoords2.set(xcoord, ycoord);

		//std::cout << pos.X / ortho_width << ", " << 1.0 - pos.Y / ortho_height << "\n";
		//std::cout << (f32)texture_block.UpperLeftCorner.X / lightmap_image->getDimension().Width
		//	<< ", " << (f32)texture_block.UpperLeftCorner.Y / lightmap_image->getDimension().Height << "\n";
		//std::cout << vtx->TCoords2.X << ", " << vtx->TCoords2.Y << "\n";
	}
}
/*
bool GenLightMaps::sphere_intersects_quad(int f_j, core::vector3df v0, u16 Radius, core::vector3df* points_)
{
	core::vector3df points[4]{points_[0],points_[1],points_[2],points_[3]};

	plane3df plane(points[0],points[1],points[2]);

	vector3df R;

	plane.getIntersectionWithLine(v0, plane.Normal, R);

	vector3df r = R - v0;

	if (r.getLength() > Radius)
		return false;

	f32 radius = sqrt(Radius * Radius - r.getLength() * r.getLength());

	polyfold* pf = geo_scene->get_total_geometry();
	poly_face* f = &pf->faces[f_j];
	core::matrix4 mRot = f->get2Dmat();

	vector3df R0 = R;

	mRot.rotateVect(R0);
	mRot.rotateVect(points[0]);
	mRot.rotateVect(points[1]);
	mRot.rotateVect(points[2]);
	mRot.rotateVect(points[3]);



	core::line2df 
}*/
rect<f32> GenLightMaps::get_2D_bounding_box(poly_face* f)
{
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
				face_bbox.UpperLeftCorner.X =
					fmin(f->loops[i].min_x, face_bbox.UpperLeftCorner.X);
				face_bbox.UpperLeftCorner.Y =
					fmin(f->loops[i].min_z, face_bbox.UpperLeftCorner.Y);
				face_bbox.LowerRightCorner.X =
					fmax(f->loops[i].max_x, face_bbox.LowerRightCorner.X);
				face_bbox.LowerRightCorner.Y =
					fmax(f->loops[i].max_z, face_bbox.LowerRightCorner.Y);
			}
		}
	}

	return face_bbox;
}


void GenLightMaps::calc_lightmaps(int method)
{
	core::vector3df pos;
	core::vector3df dir = vector3df(100, 0, 0);

	//int LightRadius=256;

	std::vector<Reflected_LightSceneNode*> lights;

	for (int i = 0; i < geo_scene->getSceneNodes().size(); i++)
	{
		reflect::TypeDescriptor_Struct* td = geo_scene->getSceneNodes()[i]->GetDynamicReflection();

		//std::cout << td->name << "\n";

		if (strcmp(td->name, "Reflected_LightSceneNode") == 0)
		{
			lights.push_back((Reflected_LightSceneNode*)(geo_scene->getSceneNodes()[i]));
		}
	}

	//std::cout << "found "<<lights.size()<<" lights!\n";

	//std::cout << "Light Radius = " << LightRadius << "\n";

	std::vector<TextureMaterial> material_groups;

	material_groups = geo_scene->edit_meshnode_interface.getMaterialsUsed();
	polyfold* pf = geo_scene->get_total_geometry();

	m_graph.points.clear();
	m_graph.lines.clear();
	LineHolder nograph;

	for (int i = 0; i < material_groups.size(); i++)
	{
		core::dimension2d<u32> lm_size(material_groups[i].lightmap_size, material_groups[i].lightmap_size);
		
		video::IImage* lightmap_img = driver->createImage(irr::video::ECF_A8R8G8B8, lm_size);

		for (int j = 0; j < material_groups[i].faces.size(); j++)
		{
			int f_j = material_groups[i].faces[j];
			poly_face* f = &geo_scene->get_total_geometry()->faces[f_j];

			vector3df points[5];

			rect<f32> bbox_2D = get_2D_bounding_box(f);

			get_bounding_quad(geo_scene->get_total_geometry(), f, bbox_2D, points);

			points[4] = points[0] + points[1] + points[2] + points[3];
			points[4] /= 4;

			matrix4 VIEW, PROJ;
			VIEW.buildCameraLookAtMatrixLH(points[4] + f->m_normal * 100, points[4],
				is_parallel_normal(f->m_normal, vector3df(0.0, 1.0, 0.0)) ? vector3df(0.5, 1.0, 0.0) : vector3df(0.0, 1.0, 0.0));

			f32 ortho_size = calculate_actual_ortho_size(VIEW, points, bbox_2D.getWidth(), bbox_2D.getHeight());

			PROJ.buildProjectionMatrixOrthoLH(ortho_size, ortho_size, 0, 10000);

			core::matrix4 tcoord_transform;
			tcoord_transform.setbyproduct_nocheck(PROJ, VIEW);

			MeshBuffer_Chunk chunk = geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(f_j);
			set_mesh_tcoords(chunk, tcoord_transform, material_groups[i].blocks[j], bbox_2D.getWidth(), bbox_2D.getHeight(), lightmap_img->getDimension());

			chunk = geo_scene->final_meshnode_interface.get_mesh_buffer_by_face(f_j);
			set_mesh_tcoords(chunk, tcoord_transform, material_groups[i].blocks[j], bbox_2D.getWidth(), bbox_2D.getHeight(), lightmap_img->getDimension());

			video::IImage* lightmap_section = driver->createImage(irr::video::ECF_A8R8G8B8, dimension2du(f->lightmap_res, f->lightmap_res));
			video::SColor color;
			for (int i = 0; i < f->lightmap_res; i++)
				for (int j = 0; j < f->lightmap_res; j++)
				{
					color.set(255, 0, 0, 0);
					lightmap_section->setPixel(i, j, color);
				}
			switch (method)
			{
			case 0:
				for (Reflected_LightSceneNode* a_light : lights)
				{
					current_light_pos = a_light->getPosition();
					current_light_radius = a_light->light_radius;

					render_face(a_light->getPosition(), f_j, lightmap_section, a_light->light_radius, VIEW, PROJ, points, bbox_2D, nograph);
				} break;
			case 1:
				for (int mbi = 0; mbi < geo_scene->final_meshnode_interface.getMesh()->getMeshBufferCount(); mbi++)
				{
					IMeshBuffer* mb = geo_scene->final_meshnode_interface.getMesh()->getMeshBuffer(mbi);
				}
				break;
			}

			vector2di block_pos(material_groups[i].blocks[j].UpperLeftCorner.X, material_groups[i].blocks[j].UpperLeftCorner.Y);
			
			lightmap_section->copyTo(lightmap_img, block_pos);
			lightmap_section->drop();
		}

		video::ITexture* lightmap_texture = driver->addTexture("lightmap", lightmap_img);
		lightmap_textures2.push_back(lightmap_texture);

		irr::u8* imgDataPtr = (irr::u8*)lightmap_img->lock();

		std::stringstream ss;
		ss << "../media/bmpTest" << i << ".bmp";

		generateBitmapImage(imgDataPtr, lm_size.Width, lm_size.Height, ss.str().c_str());

		lightmap_img->unlock();

		lightmap_img->drop();
	}

	for (int i = 0; i < material_groups.size(); i++)
	{
		for (int j = 0; j < material_groups[i].faces.size(); j++)
		{
			int f_j = material_groups[i].faces[j];
			MeshBuffer_Chunk chunk = geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(f_j);
			chunk.buffer->getMaterial().setTexture(1, lightmap_textures2[i]);

			MeshBuffer_Chunk chunk_fin = geo_scene->final_meshnode_interface.get_mesh_buffer_by_face(f_j);
			chunk_fin.buffer->getMaterial().setTexture(1, lightmap_textures2[i]);
		}
	}

	geo_scene->getMeshNode()->copyMaterials();

}

void GenLightMaps::addDrawLines(LineHolder& graph)
{
	graph.lines = m_graph.lines;
	graph.points = m_graph.points;
}

u16 GenLightMaps::guessLMres(int f_j)
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

	return std::min(256, std::max(8 , ret / 4 ));
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
		coords = rect<u16>(origin, dimension2d<u16>(size, size));

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
bool GenLightMaps::fill(std::vector<int>::iterator& faces_it, std::vector<int>::iterator faces_end, out_Type out, const TextureMaterial& copy_from)
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

void GenLightMaps::divideMaterialGroups(std::vector<TextureMaterial>& material_groups)
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

		while (!fill(faces_it, faces_end, ret_back, *in_it))
		{
			++ret_back;
		}

		++ret_back;
	}

	//std::cout << ret.size() << " new material groups\n";

	material_groups = ret;
}




/*
		//COMPUTE A HOMOGRAPHY

		Eigen::Matrix<double, 8, 9> A {
			{ -x[0], -y[0], -1, 0, 0, 0, x[0] * u[0], y[0] * u[0], u[0] },
			{ 0, 0, 0, -x[0], -y[0], -1, x[0] * v[0], y[0] * v[0], v[0] },
			{ -x[1], -y[1], -1, 0, 0, 0, x[1] * u[1], y[1] * u[1], u[1] },
			{ 0, 0, 0, -x[1], -y[1], -1, x[1] * v[1], y[1] * v[1], v[1] },
			{ -x[2], -y[2], -1, 0, 0, 0, x[2] * u[2], y[2] * u[2], u[2] },
			{ 0, 0, 0, -x[2], -y[2], -1, x[2] * v[2], y[2] * v[2], v[2] },
			{ -x[3], -y[3], -1, 0, 0, 0, x[3] * u[3], y[3] * u[3], u[3] },
			{ 0, 0, 0, -x[3], -y[3], -1, x[3] * v[3], y[3] * v[3], v[3] }
		};

		Eigen::JacobiSVD<Eigen::Matrix<double, 8, 9>, Eigen::ComputeFullU | Eigen::ComputeFullV> svd(A);

		std::cout << "Its singular values are:" << std::endl << svd.singularValues() << std::endl;


		Eigen::Vector<double, 9> p = svd.matrixV().col(8);

		Eigen::Matrix<double, 3, 3> P{
			{ p[0], p[1], p[2]},
			{ p[3], p[4], p[5] },
			{ p[6], p[7], p[8] },
		};

		Eigen::Vector3d rvec = P * Eigen::Vector3d(x[0], y[0], 1);
		rvec /= rvec[2];
		std::cout << "vector 0 = \n" << rvec << "\n";

		rvec = P * Eigen::Vector3d(x[1], y[1], 1);
		rvec /= rvec[2];
		std::cout << "vector 1 = \n" << rvec << "\n";

		rvec = P * Eigen::Vector3d(x[2], y[2], 1);
		rvec /= rvec[2];
		std::cout << "vector 2 = \n" << rvec << "\n";

		rvec = P * Eigen::Vector3d(x[3], y[3], 1);
		rvec /= rvec[2];
		std::cout << "vector 3 = \n" << rvec << "\n";

		//std::cout << "Its right singular vectors are the columns of the thin V matrix:" << std::endl << svd.matrixV() << std::endl;
		//std::cout << "---\n";

		}*/

