#ifndef _LIGHTMAPS_H_
#define _LIGHTMAPS_H_

#include <irrlicht.h>
#include "csg_classes.h"
#include "BufferManager.h"

class geometry_scene;



class LightMap_Plate
{
public:
	LightMap_Plate() {}
	~LightMap_Plate() {
		if (Buffer) delete Buffer;
	}

	//virtual const core::aabbox3df& getBoundingBox() const;
	//virtual void OnRegisterSceneNode();

	void set_buffer(const scene::SViewFrustum &frustum, core::plane3df plane);
	void render(video::IVideoDriver* driver, video::SMaterial& material);
	//virtual void render_special(video::SMaterial& material);

	//virtual void Select();
	//virtual void UnSelect();

	//core::rect<s32> GetVisibleRectangle(TestPanel* viewPanel);

	video::ITexture* m_texture = NULL;
	scene::SMeshBuffer* Buffer = NULL;

};

class GenLightMaps
{
public:
	GenLightMaps(geometry_scene*, scene::ISceneManager*, video::IVideoDriver*, video::E_MATERIAL_TYPE projectionMaterialType);
	~GenLightMaps();

	void init();
	//void render_face(core::vector3df v0, int f_i, video::IImage* lightmap_section, int Radius, core::dimension2du lightmap_dim, LineHolder& graph);

	void calc_lightmaps(int method);
	void calc_lightmap_tcoords();

	video::ITexture* getTexture() {
		return Texture;
	}

	video::ITexture* getIntermediateTexture()
	{
		return intermediate_texture;
	}

	//video::ITexture* getOrthoTexture() {
	//	return ortho_texture;
	//}

	
	core::matrix4 getProjectionMatrix() {
		//return mProjection;
		return m_PROJECTION;
	}

	void setDynamicLightMaterial(video::E_MATERIAL_TYPE mat)
	{
		m_DynamicLightMaterialType = mat;
	}
	void setPathTracingMaterial(video::E_MATERIAL_TYPE mat)
	{
		m_PathTracingMaterialType = mat;
	}

	core::vector3df GetCurrentLightPos()
	{
		return current_light_pos;
	}
	
	u16 GetCurrentLightRadius()
	{
		return current_light_radius;
	}
	
	int* GetNTriangles();
	core::vector3df* GetVertices();
	core::vector3df* GetNormals();

	core::matrix4 getViewMatrix() {
		//return mView;
		return m_VIEW;
	}

	void addDrawLines(LineHolder& graph);

	video::ITexture* getLightmap(int n)
	{
		if (n < lightmap_textures.size())
			return lightmap_textures[n];

		return NULL;
	}

	u16 getMaxLMTexSize()
	{
		return 1024;
	}

	u16 guessLMres(int f_j);
	void divideMaterialGroups(std::vector<TextureMaterial>& material_groups);

	static core::rect<f32> get_2D_bounding_box(poly_face* f);
	static void get_bounding_quad(polyfold* pf, poly_face* f, core::rect<f32> bbox, core::vector3df* points);

private:

	void calculate_optimal_view_frustum(core::vector3df v0, core::vector3df points[4], bool reverse_triangles, core::dimension2d<u32> dim, core::matrix4& proj_out, core::matrix4& view_out);
	void render_light(video::ITexture*, int f_j, const core::matrix4& view, const core::matrix4 proj);
	f32 calculate_actual_ortho_size(const core::matrix4& VIEW, core::vector3df points[4], u16 ortho_width, u16 ortho_height);
	void set_mesh_tcoords(const MeshBuffer_Chunk& chunk, const core::matrix4& transform, core::rect<u16> texture_block, u16 ortho_width, u16 ortho_height, core::dimension2du lightmap_dim);
	//bool get_bounding_quadrilateral(int f_j, u16 Radius, core::vector3df* points_out, f32& ortho_width, f32& ortho_height, LineHolder& graph);
	//bool sphere_intersects_quad(int f_j, core::vector3df v0, u16 Radius, core::vector3df* points);
	bool light_intersects_quad(int f_j, core::vector3df v0, u16 Radius, const core::rect<f32>& face_bbox, LineHolder& graph);

	core::vector3df current_light_pos;
	u16 current_light_radius;

	void render_face(core::vector3df v0, int f_j, video::IImage* lightmap_section, int Radius, const core::matrix4& ORTHO_VIEW, const core::matrix4& ORTHO_PROJ, core::vector3df* points, const core::rect<f32>& bbox_2D, LineHolder& graph);

	//void render_face2(core::vector3df v0, int f_j, video::IImage* lightmap_section, int Radius, const core::matrix4& ORTHO_VIEW, const core::matrix4& ORTHO_PROJ, core::vector3df* points, const core::rect<f32>& bbox_2D, LineHolder& graph);

	template<typename out_Type>
	bool fill(std::vector<int>::iterator& faces_it, std::vector<int>::iterator faces_end, out_Type out, const TextureMaterial& copy_from);

	geometry_scene* geo_scene = NULL;
	scene::ISceneManager* smgr = NULL;
	video::IVideoDriver* driver = NULL;

	video::ITexture* intermediate_texture = NULL;
	video::ITexture* Texture = NULL;
	//video::ITexture* ortho_texture = NULL;
	video::ITexture* black_texture = NULL;
	video::ITexture* white_texture = NULL;

	LightMap_Plate* m_plate = NULL;

	core::matrix4 m_VIEW;
	core::matrix4 m_PROJECTION;

	//core::matrix4 mProjection;
	//core::matrix4 mView;

	video::E_MATERIAL_TYPE m_projectionMaterialType;
	video::E_MATERIAL_TYPE m_DynamicLightMaterialType;
	video::E_MATERIAL_TYPE m_PathTracingMaterialType;

	LineHolder m_graph;

	std::vector<video::ITexture*> lightmap_textures;
	std::vector<video::ITexture*> lightmap_textures2;
	std::vector<video::IImage*> lightmap_images;

	int m_nTriangles = 0;
	core::vector3df* m_Vertices = NULL;
	core::vector3df* m_Normals = NULL;
};

class LightMaps_Tool
{
public:
	//LightMaps_Tool()
	static void init(GenLightMaps* lightmaps) {
		m_lightmaps = lightmaps;
	}

	static GenLightMaps* getLightmaps() {
		return m_lightmaps;
	}

private:
	static GenLightMaps* m_lightmaps;
};

#endif